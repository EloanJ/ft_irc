/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejonsery <ejonsery@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 14:30:33 by vduarte           #+#    #+#             */
/*   Updated: 2026/01/13 15:42:39 by ejonsery         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

static bool g_stopSignal = false;

Server::Server(char* port, std::string pswd, std::string name) : _name(name), _password(pswd)
{
	if (this->verifInput(port))
		exit(1);
	this->_serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_serverfd < 0)
	{
		std::cerr<<RED<<"socket error"<<RST<<std::endl;
		exit(1);
	}
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(atoi(port));
	serverAddress.sin_addr.s_addr = INADDR_ANY;
 	bind(this->_serverfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (listen(this->_serverfd, -1) < 0)
	{
		std::cerr<<RED<<"listen error"<<RST<<std::endl;
		close(this->_serverfd);
		exit(1);
	}
	this->_fds.push_back((pollfd){this->_serverfd, POLLIN, 0}); // fd, event, revents
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = Server::signalServer;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGINT);
	sigaddset(&act.sa_mask, SIGQUIT);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	this->_sdate = time(NULL);
}

Server::~Server()
{
	if (this->_channels.size() > 0)
	{
		for (std::map<std::string, Channel *>::iterator i = this->_channels.begin(); i != this->_channels.end(); i++)
		{
			if (i->second != NULL)
				delete i->second;
		}
		this->_channels.clear();
	}
	if (this->_clients.size() > 0)
	{
		for (std::map<int, Client *>::iterator i = this->_clients.begin(); i != this->_clients.end(); i++)
		{
			if (i->second != NULL)
				delete i->second;
		}
		this->_clients.clear();
	}
}

void Server::signalServer(int sig)
{
	if (sig == SIGINT || sig == SIGQUIT)
	{
		g_stopSignal = 1;
		std::cout<<ORANGE<<"Trying to close the server"<<RST<<std::endl;
	}
}

/*
 - Verification du port :
  - Plages de 0 a 1023 : reserves au systeme
  - Plages de 1024 a 43151 : applications mais utilisables
  - Plages de 43152 a 65535 : utilisables pour tous les usages
 */
int Server::verifInput(std::string port)
{
	if (port.size() < 3 || port.size() > 4 || port[0] == '-')
	{
		std::cerr<<RED<<"Please provide a valid port"<<RST<<std::endl;
		return 1;
	}
	for (size_t i = 0; i < port.size(); i++)
	{
		if (!isdigit(port[i]))
		{
			std::cerr<<RED<<"Please provide a valid port"<<RST<<std::endl;
			return 1;
		}
	}
	if (atoi(port.c_str()) < 1023 || atoi(port.c_str()) > 65535)
	{
		std::cerr<<RED<<"Please provide a valid port"<<RST<<std::endl;
		return 1;
	}
	return 0;
}

int	Server::startServer()
{
	std::cout<<GREEN<<"Server starting..."<<RST<<std::endl;
	this->_lastPing = std::time(NULL);
	while (true)
	{
		if (g_stopSignal)
		{
			close(this->_serverfd);
			return 0;
		}
		std::cout<<ORANGE<<"-- IN LOOP --"<<RST<<std::endl;
		if (poll(&this->_fds[0], this->_fds.size(), -1) < 0)
		{
			if (!g_stopSignal)
				std::cerr<<RED<<"poll error"<<RST<<std::endl;
			else
			{
				std::cout<<RED<<"Server closing..."<<RST<<std::endl;
				close(this->_serverfd);
				return (0);
			}
		}
		time_t now = std::time(NULL);
		std::cout<<BOLDWHITE<<"Nombre de fd client(s) + fd server : "<<this->_fds.size()<<RST<<std::endl;
		std::cout<<GREEN<<"Time ping : "<<now - this->_lastPing<<RST<<std::endl;
		if (now - this->_lastPing >= 30)
		{
			for (size_t j = 0; j < this->_fds.size(); j++)
			{
				if (this->_fds[j].fd != this->_serverfd)
				{
					std::string ping = "PING :"+this->_name+"\r\n";
					send(this->_fds[j].fd, ping.c_str(), ping.size(), 0);
					std::cout<<GREEN<<"PING send to : "<<this->_fds[j].fd<<RST<<std::endl;
				}
			}
			this->_lastPing = now;
		}
		for (size_t i = 0; i < this->_fds.size(); i++)
		{
			if (this->_fds[i].revents & POLLIN)
			{
				if (this->_fds[i].fd == this->_serverfd)
				{
					int c_fd = accept(this->_serverfd, NULL, NULL);
					std::cout << BOLDMAGENTA << "Client connected : " << c_fd << RST << std::endl;
					if (c_fd < 1)
						std::cerr<<RED<<"accept error"<<RST<<std::endl;
					this->_fds.push_back((pollfd){c_fd, POLLIN, 0});
					this->_clients.insert(std::make_pair(c_fd, new Client(c_fd)));
				}
				else
				{
					Client *clt = this->findClient(this->_fds[i].fd);
					if (!clt)
						std::cout<<RED<<"FIND CLIENT : NO CLIENT FOR CMDS\n";
					std::string totalread;
					char buffer[1024];
					bzero(buffer, sizeof(buffer));
					int n = 0;
					while ((n = recv(this->_fds[i].fd, buffer, sizeof(buffer), 0)) > 0)
					{
						if (n != sizeof(buffer))
						{
							buffer[n] = '\0';
							totalread += buffer;
							break;
						}
						totalread += buffer;
						bzero(buffer, sizeof(buffer));
					}
					std::cout<<CYAN<<"Read : "<<totalread<<RST;
					if (n <= 0)
					{
						std::cout << BOLDMAGENTA << "Client disconected : " << this->_fds[i].fd << RST << std::endl;
						this->serverQuit(this->_fds[i].fd, "");
						i--;							
					}
					else
					{
						size_t k = 0;
						while (k != totalread.size() && k != std::string::npos)
						{
							size_t end = totalread.find('\n', k);
							if (end == std::string::npos)
								end = totalread.find('\r', k);
							if (end != std::string::npos)
							{
								clt->addCmd(totalread.substr(k, end - k));
								while (totalread[end] != '\0' && (totalread[end] == '\r' || totalread[end] == '\n'))
									end++;
								k = end;
							}
							else
							{
								clt->addCmd(totalread.substr(k, totalread.size() - k));
								while (totalread[k] != '\0' && (totalread[k] == '\r' || totalread[k] == '\n'))
									k++;
								k = totalread.size();
							}
						}
						if (clt->completeCmd())
							this->commandDispatcher(clt, i);
					}
				}
			}
		}
	}
}


void Server::commandDispatcher(Client *clt, int i)
{
	int res = 0;
	std::list<std::string>& clt_cmds = clt->getCmds();
	for (std::list<std::string>::iterator it = clt_cmds.begin(); it != clt_cmds.end(); it ++)
	{
		if ((*it).find("PASS") == 0)
		{
			if ((res = createClient(this->_fds[i].fd, (*it), 0)) < 0)
				break;
		}
		else if ((*it).find("NICK") == 0)
		{
			if ((res = createClient(this->_fds[i].fd, (*it), 1)) < 0)
				break;
		}
		else if ((*it).find("USER") == 0)
		{
			if ((res = createClient(this->_fds[i].fd, (*it), 2)) < 0)
				break;
		}
		else if ((*it).find("PING") == 0) 
			serverPingPong(this->_fds[i].fd, true);
		else if ((*it).find("PONG") == 0)
			serverPingPong(this->_fds[i].fd, false);
		else if ((*it).find("JOIN") == 0)
			channelJoin(this->_fds[i].fd, (*it));
		else if ((*it).find("INVITE") == 0)
			channelInvite(this->_fds[i].fd, (*it));
		else if ((*it).find("PRIVMSG") == 0)
			msgBroadcast(this->_fds[i].fd, (*it));
		else if ((*it).find("TOPIC") == 0)
			channelTopic(this->_fds[i].fd, (*it));
		else if ((*it).find("PART") == 0)
			channelPart(this->_fds[i].fd, (*it));
		else if ((*it).find("KICK") == 0)
			channelKick(this->_fds[i].fd, (*it));
		else if ((*it).find("MODE") == 0)
			channelMode(this->_fds[i].fd, (*it));
		else if ((*it).find("QUIT") == 0)
		{
			serverQuit(this->_fds[i].fd, (*it));
			res = -1;
			break;
		}
	}
	if (res >= 0)
		clt->clearCmd();
}

int Server::createClient(int fd, std::string cmd, int step)
{
	Client *clt = this->findClient(fd);
	std::cout<<GREY<<"step :"<<step<<"creatStep :"<<clt->getCreateStep()<<RST<<std::endl;
	if (!clt)
	{
		std::cerr<<RED<<"CREATE CLIENT : NO CLIENT FOR CMDS\n"<<RST;
		return -1;
	}
	if (step != clt->getCreateStep())
	{
		std::string ts = ":"+this->_name+" 461 :Not enough parameters\r\n";
		send(fd, ts.c_str(), ts.size(), 0);
		clt->setAuth(true);
		clt->setCreateStep(-1);
		step = -1;
	}
	switch (step)
	{
		case 0:
			clt->setPassword(cmd, this->_password);
			break;
		case 1:
			clt->setNickname(cmd);
			break;
		case 2:
			clt->setUsernameServername(cmd);
			break;
	}
	if (clt->getCreateStep() == 2)
	{
		if (clt->getNickname().size() == 0)
		{
			std::string ts = ":"+this->_name+" 431 "+clt->getNickname()+" "+clt->getNickname()+" :No nickname given\r\n";
			send(fd, ts.c_str(), ts.size(), 0);
			clt->setCreateStep(-1);
		}
		else if (this->findClient(clt->getNickname()) != NULL && this->findDuplicata(clt, clt->getNickname()))
		{
			std::string ts = ":"+this->_name+" 443 "+clt->getNickname()+" "+clt->getNickname()+" :Nickname is already in use\r\n";
			send(fd, ts.c_str(), ts.size(), 0);
			clt->setCreateStep(-1);
		}		
	}
	if (clt->getCreateStep() < 0)
		return (badLogin(clt, fd));
	else if (clt->getCreateStep() == 3)
	{
		std::string m1 = ":" + this->_name + " 001 "+clt->getNickname()+" :Welcome to the Internet Relay Chat Network, "+clt->getNickname()+"\r\n";
		std::string m2 = ":" + this->_name + " 002 "+clt->getNickname()+" :Your host is " + this->_name + ", running version 4.2\r\n";
		std::string m3 = ":" + this->_name + " 003 "+clt->getNickname()+" :This server was created "+ctime(&(this->_sdate))+"\r\n";
		std::string m4 = ":" + this->_name + " 004 "+clt->getNickname()+ " " + this->_name + " 4.2 - itkol\r\n";
		send(fd, m1.c_str(), m1.size(), 0);
		send(fd, m2.c_str(), m2.size(), 0);
		send(fd, m3.c_str(), m3.size(), 0);
		send(fd, m4.c_str(), m4.size(), 0);
		std::cout<<YELLOW<<"Client: "<<clt->getNickname()<<" client created\n"<<RST;
	}
	return clt->getCreateStep();
}

int Server::badLogin(Client *clt, int fd)
{
	if (!clt->isAuth() && clt->getCreateStep() == -1)
	{
		std::string ts = ":"+this->_name+" - 464 :Password incorrect\r\n";
		send(fd, ts.c_str(), ts.size(), 0);
	}
	else if (clt->getCreateStep() == -2)
	{
		std::string ts = ":"+this->_name+" 443 "+clt->getNickname()+" "+clt->getNickname()+" :Nickname is already in use\r\n";
		send(fd, ts.c_str(), ts.size(), 0);
	}
	else if (clt->getCreateStep() == -3)
	{
		std::string ts = ":"+this->_name+" 461 :Not enough parameters\r\n";
		send(fd, ts.c_str(), ts.size(), 0);
	}
	for (std::map<int, Client *>::iterator i = this->_clients.begin(); i != this->_clients.end();i++)
	{
		if (((*i).second) != NULL && (*i).second->getFd() == fd)
		{
			delete ((*i).second);
			this->_clients.erase(fd);
			for (size_t j = 0; j < this->_fds.size(); j++)
			{
				if (this->_fds[j].fd == fd)
				{
					this->_fds.erase(this->_fds.begin() + j);
					break;
				}
			}
			close(fd);
			return -1;
		}
	}
	return -1;
}

bool Server::findDuplicata(Client *clt, std::string name)
{
	for (std::map<int, Client *>::iterator it = this->_clients.begin(); it !=this->_clients.end(); it++)
	{
		if (it->second->getNickname() == name && it->second != clt)
			return true;
	}
	return false;
}

void Server::serverPingPong(int fd, bool pingorpong)
{
	if (!pingorpong)
		std::cout<<GREEN<<"PONG from : "<<fd<<RST<<std::endl;
	else if (pingorpong)
	{
		std::string ping = "PONG :"+this->_name+"\r\n";
		send(fd, ping.c_str(), ping.size(), 0);
		std::cout<<GREEN<<"PONG send to : "<<fd<<RST<<std::endl;
	}
}

Channel *Server::findChannel(std::string name)
{
	std::map<std::string, Channel*>::iterator it = this->_channels.find(name);
	
	if (it != this->_channels.end())
		return it->second;
	return NULL;
}

Client *Server::findClient(int fd)
{
	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it != this->_clients.end())
		return it->second;
	return NULL;
}

Client *Server::findClient(std::string name)
{
	for (std::map<int, Client *>::iterator it = this->_clients.begin(); it !=this->_clients.end(); it++)
	{
		if (it->second->getNickname() == name)
			return it->second;
	}
	return NULL;
}

void Server::msgBroadcast(int fd, std::string msg)
{
	std::string chname = "";
	std::string submsg = "";
	size_t s_dp = msg.find(":");
	size_t e_msg = msg.find("\r");
	if (e_msg == std::string::npos)
		e_msg = msg.find("\n");
	if (e_msg != std::string::npos)
		chname = msg.substr(8, (s_dp - 1) - 8);
	Channel	*ch = findChannel(chname);
	Client *clt_send = findClient(fd);
	Client	*clt_rec = findClient(chname);
	if (clt_send && ch && e_msg != std::string::npos)
	{
		if (!ch->isInChannel(clt_send))
		{
			std::string to_send = ":"+this->_name+" 442 "+chname+" :You're not on that channel\r\n";
			send(fd, to_send.c_str(), to_send.size(), 0);
            return;
		}
		submsg = msg.substr(s_dp + 1, (e_msg - 1) - s_dp);
		std::string tosend = ":"+clt_send->getNickname()+"!"+clt_send->getNickname()+"@"+clt_send->getSevname()+" PRIVMSG "+chname+" :"+submsg+"\r\n";
		ch->sendToAll(tosend, fd);
		if (submsg.find('!') == 0)
			this->_bot.exclamationCommand(ch, submsg);
	}
	else if (clt_rec && e_msg != std::string::npos)
	{
		if (clt_send)
		{
			std::string tosend = ":"+clt_send->getNickname()+"!"+clt_rec->getNickname()+"@"+clt_rec->getSevname()+" PRIVMSG "+chname+" :"+msg.substr(s_dp + 1, (e_msg - 1) - s_dp)+"\r\n";
			int n = send(clt_rec->getFd(), tosend.c_str(), tosend.size(), 0);
			std::cout<<YELLOW<<"Private Message : "<<clt_send->getUsername()<<" to "<<clt_rec->getUsername()<<RST<<std::endl;
			std::cout<<YELLOW<<"Private Data : "<<n<<tosend<<RST<<std::endl;
		}
	}
	else if (!clt_rec && clt_send)
	{
		std::string ts = ":"+this->_name+" 401 "+clt_send->getNickname()+" :No such nick/channel\r\n";
		send(fd, ts.c_str(), ts.size(), 0);
	}
}

void Server::channelJoin(int fd, std::string cmd)
{
	char		characters[4] = {'&', '#', '+', '!'};
	std::string ch_name = "";
	std::string key = "";

	for (int i = 0; i < 4; i++)
	{
		char c = characters[i];
		if (cmd.find(c) != std::string::npos)
		{
			size_t chname_sta = cmd.find(c);
			size_t chname_end = cmd.find(' ', chname_sta);
			size_t key_end = cmd.find('\r');
			if (key_end == std::string::npos)
				key_end = cmd.find('\n');
			if (chname_end != std::string::npos && chname_end < key_end)
			{
				ch_name = cmd.substr(chname_sta, (chname_end - chname_sta));
				if (key_end != std::string::npos)
				{
					key = cmd.substr((chname_end + 1), (key_end - (chname_end + 1)));
					break;
				}			
			}
			else
			{
				chname_end = cmd.find('\r');
				if (chname_end == std::string::npos)
					chname_end = cmd.find('\n');
				if (chname_end != std::string::npos)
				{
					ch_name = cmd.substr(chname_sta, chname_end - chname_sta);
					break;
				}				
			}
		}
	}
	std::cout<<GREY<<"Join Channel -- Step 1\n"<<"Channel name : ["<<ch_name<<"]\n"<<"Channel Key : ["+key+"]\n"<<RST;
	Client* clt = findClient(fd);
	Channel* ch = findChannel(ch_name);
	if (clt && ch_name.size() == 0)
	{
		std::string ts = ":"+this->_name+" 461 "+clt->getNickname()+" JOIN :Not enough paramaters\r\n";
		send(fd, ts.c_str(), ts.size(), 0);
	}
	else if (clt && ch)
		ch->joinChannel(clt, key);
	else if (clt && !ch)
	{
		Channel *newCh = new Channel(ch_name, clt, &this->_name);
		this->_channels.insert(std::make_pair(ch_name, newCh));
		newCh->joinChannel(clt, key);
		std::cout<<GREY<<"Client : ["+clt->getNickname()+"] created ["+newCh->getChName()+"]\n"<<RST;	
	}
}

void Server::channelPart(int fd, std::string cmd)
{
	std::cout<<BOLDYELLOW<<"channelPart called"<<RST<<std::endl;
	std::string ch_name = "";
	std::string reason = "";

	size_t part_pos = cmd.find("PART ");
	if (part_pos != std::string::npos)
	{
		size_t chname_start = part_pos + 5;
		size_t chname_end = cmd.find(':', chname_start);
		if (chname_end == std::string::npos)
			chname_end = cmd.find('\r', chname_start);
		if (chname_end == std::string::npos)
			chname_end = cmd.find('\n', chname_start);
		if (chname_end != std::string::npos)
		{
			ch_name = cmd.substr(chname_start, chname_end - chname_start);
			size_t end = ch_name.find_last_not_of(" \t\r\n");
			if (end != std::string::npos)
			{
				ch_name = ch_name.substr(0, end + 1);
				size_t r_st = cmd.find(':', end);
				if (r_st != std::string::npos)
				{
					size_t r_end = cmd.find('\r', r_st);
					if (r_end == std::string::npos)
						r_end = cmd.find('\n', r_st);
					if (r_end != std::string::npos)
						reason = cmd.substr(r_st + 1, (r_end - r_st) - 1);
				}
			}
		}
	}
	std::cout << "ch_name brut : [" << ch_name << "] taille: " << ch_name.size() << std::endl;
	if (ch_name.size() != 0)
	{
		Channel	*ch = findChannel(ch_name);
		Client	*clt = findClient(fd);
		std::cout<<"Client :"<<clt->getNickname()<<std::endl;
		if (!clt)
			return ;
		if (!ch)
		{
			std::cout<<"ch not find without #"<<std::endl;
			std::string ts = ":"+this->_name+" 403 "+clt->getNickname()+" "+ch_name+" :No such channel\r\n";
			send(fd, ts.c_str(), ts.size(), 0);
			return ;
		}
		if (!ch->isInChannel(clt))
		{
			std::string to_send = ":"+this->_name+" 442 "+ch_name+" :You're not on that channel\r\n";
			send(fd, to_send.c_str(), to_send.size(), 0);
			return ;
		}
		ch->leaveChannel(clt, reason);
	}
	else
	{
		Client *clt = findClient(fd);
        if (clt)
		{
			std::string ts = ":"+this->_name+" 461 "+clt->getNickname()+" PART :Not enough paramaters\r\n";
			send(fd, ts.c_str(), ts.size(), 0);
		}
	}
}

void Server::channelKick(int fd, std::string cmd)
{
	char		characters[4] = {'&', '#', '+', '!'};
	std::string ch_name = "";
	std::string cltk_name = "";
	std::string reason = "Because the operator's supreme authority cannot be contradicted.";

	for (int i = 0; i < 4; i++)
	{
		char c = characters[i];
		if (cmd.find(c) != std::string::npos)
		{
			size_t ch_st = cmd.find(c);
			size_t ch_end = cmd.find(' ', ch_st);
			if (ch_end != std::string::npos)
			{
				ch_name = cmd.substr(ch_st, ch_end - ch_st);
				size_t n_st = ch_end + 1;
				size_t n_end = cmd.find(' ', n_st);
				if (n_end != std::string::npos)
				{
					cltk_name = cmd.substr(n_st, n_end - n_st);
					size_t r_st = cmd.find(':', n_end);
					if (r_st != std::string::npos)
					{
						size_t r_end = cmd.find('\r', r_st);
						if (r_end == std::string::npos)
							r_end = cmd.find('\n', r_st);
						if (r_end != std::string::npos)
							reason = cmd.substr(r_st + 1, (r_end - r_st) - 1);						
					}

				}
			}
			break;
		}
	}
	std::cout<<GREY<<"KICK DEBUG\nChannel name : ["+ch_name+"]\nUsername : ["+cltk_name+"]\nReason : ["+reason+"]\n"<<RST;
	if (ch_name.size() > 0 && cltk_name.size() > 0)
	{
		Channel *chn = this->findChannel(ch_name);
		Client *clt_op = this->findClient(fd);
		Client *clt_tk = this->findClient(cltk_name);
		if (!clt_tk && clt_op)
		{
			std::string ts = ":"+this->_name+" 401 "+clt_op->getNickname()+" :No such nick/channel\r\n";
			send(fd, ts.c_str(), ts.size(), 0);
		}
		else if (clt_op && (!chn || !clt_op))
		{
			std::string ts = ":"+this->_name+" 461 "+clt_op->getNickname()+" KICK :Not enough paramaters\r\n";
			send(fd, ts.c_str(), ts.size(), 0);
		}
		else if (chn && clt_op && clt_tk)
			chn->kickChannel(clt_op, clt_tk, reason);
	}
}

void Server::channelInvite(int fd, std::string cmd)
{
	char		characters[4] = {'&', '#', '+', '!'};
	std::string ch_name = "";
	std::string clti_name = "";

	std::cout<<GREY<<"INVITE -- STEP 1"<<RST<<std::endl;
	for (int i = 0; i < 4; i++)
	{
		char c = characters[i];
		if (cmd.find(c) != std::string::npos)
		{
			size_t chname_end = cmd.find('\r');
			if (chname_end == std::string::npos)
				chname_end = cmd.find('\n');
			if (chname_end != std::string::npos)
			{
				int ch_start = cmd.find(c);
				ch_name = cmd.substr(ch_start, chname_end - ch_start);
				int n_start = cmd.find(" ", 0);
				int n_end = ch_start - 2;
				clti_name = cmd.substr(n_start + 1, n_end - n_start);
				break;
			}
		}
	}
	std::cout<<GREY<<"Client to invite : ["<<clti_name<<"]\nOn Channel : ["<<ch_name<<"]"<<RST<<std::endl; 
	Client *clt_snd = this->findClient(fd);
	if (clt_snd && ch_name.size() > 0 && clti_name.size() > 0)
	{
		Channel *chn = this->findChannel(ch_name);
		Client *clt_rec = this->findClient(clti_name);
		if (!chn)
			std::cout<<GREY<<"No channel found"<<RST<<std::endl;
		if (!clt_rec)
		{
			std::string ts = ":"+this->_name+" 401 "+clti_name+" :No such nick/channel\r\n";
			send(fd, ts.c_str(), ts.size(), 0);
		}
		if (chn && clt_snd && clt_rec)
			chn->inviteChannel(clt_snd, clt_rec);
	}
	else if (clt_snd && (ch_name.size() == 0 || clti_name.size() == 0))
	{
		std::string ts = ":"+this->_name+" 461 "+clt_snd->getNickname()+" INVITE :Not enough paramaters\r\n";
		send(fd, ts.c_str(), ts.size(), 0);
	}
}

void Server::channelTopic(int fd, std::string cmd)
{
	char		characters[4] = {'&', '#', '+', '!'};
	std::string ch_name = "";
	std::string ntopic = "";
	bool btopic = false;
	

	for (int i = 0; i < 4; i++)
	{
		char c = characters[i];
		if (cmd.find(c) != std::string::npos)
		{
			size_t ch_end = cmd.find(":");
			if (ch_end != std::string::npos)
			{
				btopic = !btopic;
				size_t ch_st = cmd.find(characters[i]);
				ch_name = cmd.substr(ch_st, (ch_end - ch_st - 1));
				size_t m_end = cmd.find("\r");
				if (m_end == std::string::npos)
					m_end = cmd.find("\n");
				if (m_end != std::string::npos)
					ntopic = cmd.substr(ch_end + 1, m_end - ch_end - 1);
				std::cout<<GREY<<ch_st<<" "<<ch_end<<" "<<m_end<<RST;
			}
			else
			{
				ch_end = cmd.find("\r");
				if (ch_end == std::string::npos)
					ch_end = cmd.find("\n");
				if (ch_end != std::string::npos)
				{
					size_t ch_st = cmd.find(characters[i]);
					ch_name = cmd.substr(ch_st, ch_end - ch_st);
				}
			}
			break;
		}
	}
	if (btopic)
		std::cout<<GREY<<"DEBUG TOPIC\nCHANNEL NAME : "<<ch_name<<"$\nTOPIC NAME : "<<ntopic<<RST<<std::endl;
	else
		std::cout<<GREY<<"DEBUG TOPIC\nCHANNEL NAME : "<<ch_name<<"$\nNO TOPIC NAME\n"<<RST;
	Client *clt = this->findClient(fd);
	Channel *ch = this->findChannel(ch_name);
	std::cout<<GREY<<"DEBUG CHANNEL TOPIC\n"<<clt<<" "<<ch<<std::endl;
	if (clt && ch && !btopic)
		ch->sndTopicChannel(clt);
	else if (clt && ch && btopic)
		ch->setTopicChannel(clt, ntopic);
	else if (clt && !ch && ch_name.size() == 0)
	{
		std::string ts = ":"+this->_name+" 461 "+clt->getNickname()+" TOPIC :Not enough paramaters\r\n";
		send(fd, ts.c_str(), ts.size(), 0);
	}
}

void Server::channelMode(int fd, std::string cmd)
{
	char		characters[4] = {'&', '#', '+', '!'};
	char		carmode[2] = {'+', '-'};
	std::string ch_name = "";
	std::string mode_ch = "";
	std::string tparam = "";
	for (int i = 0; i < 4; i++)
	{
		char c = characters[i];
		if (cmd.find(c) != std::string::npos)
		{
			size_t cst = cmd.find(characters[i]);
			if (cst != std::string::npos)
			{
				size_t cend = cmd.find(" ", cst);
				if (cend == std::string::npos)
					cend = cmd.find("\r", cst);
				if (cend == std::string::npos)
					cend = cmd.find("\n", cst);
				if (cend != std::string::npos)
					ch_name = cmd.substr(cst, (cend - cst));
			}
			break;
		}
	}
	for (int i = 0; i < 2; i++)
	{
		char c = carmode[i];
		if (cmd.find(c) != std::string::npos)
		{
			size_t m_st = cmd.find(c);
			size_t e_st = cmd.find(' ', m_st);
			if (e_st == std::string::npos)
			{
				e_st = cmd.find('\r');
				if (e_st == std::string::npos)
					e_st = cmd.find('\n');
				if (e_st != std::string::npos)
					mode_ch = cmd.substr(m_st, (e_st - m_st));
			}
			else
			{
				if (e_st != std::string::npos)
					mode_ch = cmd.substr(m_st, (e_st - m_st));
				size_t m_end = cmd.find('\r');
				if (m_end == std::string::npos)
					m_end = cmd.find('\n');
				if (m_end != std::string::npos)
					tparam = cmd.substr(e_st + 1, (m_end - (e_st + 1)));
			}
			break;
		}
	}
	std::cout<<GREY<<"MODE DEBUG\nCHANNEL NAME : "<<ch_name<<"\nMODE : "<<mode_ch<<"\nTPARAM : "<<tparam<<RST<<std::endl;
	Channel *ch = this->findChannel(ch_name);
	Client	*clt = this->findClient(fd);
	if (ch && clt)
		ch->modeChannel(clt, mode_ch, tparam);
	else if (!ch && clt && cmd.find(clt->getNickname()) == std::string::npos)
	{
		std::string ts = ":"+this->_name+" 403 "+clt->getNickname()+" "+ch_name+" :No such channel\r\n";
		send(clt->getFd(), ts.c_str(), ts.size(), 0);
	}
}

void Server::clearClientChannel(Client *clt)
{
	for (std::map<std::string, Channel *>::iterator i; i != this->_channels.end(); i++)
	{
		if (i->second->isInChannel(clt))
			i->second->leaveChannel(clt, "");
	}
}

void Server::serverQuit(int fd, std::string cmd)
{
	std::string msg = "";
	size_t st_msg = cmd.find(' ');
	if (st_msg == std::string::npos)
		st_msg = cmd.find('\r');
	if (st_msg == std::string::npos)
		st_msg = cmd.find('\n');
	if (st_msg != std::string::npos)
	{
		size_t end_msg = cmd.find('\r');
		if (end_msg == std::string::npos)
			end_msg = cmd.find('\n');
		if (end_msg != std::string::npos && st_msg != end_msg)
			msg = cmd.substr(st_msg + 2, end_msg - (st_msg + 2));
	}
	Client *clt = this->findClient(fd);
	if (clt != NULL)
	{
		for (std::map<std::string, Channel *>::iterator i = this->_channels.begin(); i != this->_channels.end(); i++)
		{
			if ((*i).second && (*i).second->isInChannel(clt))
				(*i).second->leaveChannel(clt, msg);
		}
		std::map<int, Client *>::iterator it = this->_clients.find(fd);
		if ((*it).second)
			delete (*it).second;
		this->_clients.erase(fd);
	}
	for (size_t j = 0; j < this->_fds.size(); j++)
	{
		if (this->_fds[j].fd == fd)
		{
			this->_fds.erase(this->_fds.begin() + j);
			close(fd);
			break;
		}
	}
}
