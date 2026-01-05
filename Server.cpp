/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 14:30:33 by vduarte           #+#    #+#             */
/*   Updated: 2026/01/05 16:12:04 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

static bool g_stopSignal = false;

Server::Server(char* port, std::string pswd, std::string name) : _name(name), _password(pswd)
{
	if (this->verifInput(port))
		return ;
	this->_serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_serverfd < 0)
		std::cerr<<RED<<"socket error"<<RST<<std::endl;
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(atoi(port));
	serverAddress.sin_addr.s_addr = INADDR_ANY;
 	bind(this->_serverfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (listen(this->_serverfd, -1) < 0)
		std::cerr<<RED<<"listen error"<<RST<<std::endl;
	this->fds.push_back((pollfd){this->_serverfd, POLLIN, 0}); // fd, event, revents
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = Server::signalServer;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGINT);
	sigaddset(&act.sa_mask, SIGQUIT);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
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
		if (poll(&this->fds[0], this->fds.size(), -1) < 0)
			std::cerr<<RED<<"poll error"<<RST<<std::endl;
		time_t now = std::time(NULL);
		std::cout<<BOLDWHITE<<"Nombre de fd client(s) + fd server : "<<this->fds.size()<<RST<<std::endl;
		std::cout<<GREEN<<"Time ping : "<<now - this->_lastPing<<RST<<std::endl;
		if (now - this->_lastPing >= 30)
		{
			for (size_t j = 0; j < this->fds.size(); j++)
			{
				if (this->fds[j].fd != this->_serverfd)
				{
					std::string ping = "PING :"+this->_name+"\r\n";
					send(this->fds[j].fd, ping.c_str(), ping.size(), 0);
					std::cout<<GREEN<<"PING send to : "<<this->fds[j].fd<<RST<<std::endl;
				}
			}
			this->_lastPing = now;
		}
		for (size_t i = 0; i < this->fds.size(); i++)
		{
			if (this->fds[i].revents & POLLIN)
			{
				if (this->fds[i].fd == this->_serverfd)
				{
					int c_fd = accept(this->_serverfd, NULL, NULL);
					if (c_fd < 1)
						std::cerr<<RED<<"accept error"<<RST<<std::endl;
					else
					{
						char clientInfo[1024];
						bzero(clientInfo, 1024);
						std::string clientInfoTotal;
						int end;
						while ((end = recv(c_fd, clientInfo, 1024, 0)) > 0)
						{
							clientInfo[end] = '\0';
							clientInfoTotal += clientInfo;							
							if (clientInfoTotal.find("USER", 0) != std::string::npos)
								break;
						}
						Client* newClient = new Client(c_fd, clientInfoTotal, this->_password);
						if (newClient && !newClient->isAuth())
						{
							this->sendMSG(c_fd, "464", newClient->getNickname(), "ERR_PASSWDMISMATCH", "Password incorrect");
							delete newClient;
							close(c_fd);
						}
						else if (newClient)
						{
							const std::string nname = newClient->getNickname();
							const std::string uname = newClient->getUsername();
							std::string m1 = ":" + this->_name + " 001 "+nname+" :Welcome to the Internet Relay Chat Network, "+nname+"!"+nname+"@host\r\n";
							std::string m2 = ":" + this->_name + " 002 "+nname+" :Your host is " + this->_name + ", running version 1.0\r\n";
							std::string m3 = ":" + this->_name + " 003 "+nname+" :This server was created Dec 4 2025\r\n";
							std::string m4 = ":" + this->_name + " 004 "+nname+ " " + this->_name + " 1.0 itkol\r\n";
							std::cout<<YELLOW<<"Client: "<<newClient->getUsername()<<" accepted"<<RST<<std::endl;
							send(c_fd, m1.c_str(), m1.size(), 0);
							send(c_fd, m2.c_str(), m2.size(), 0);
							send(c_fd, m3.c_str(), m3.size(), 0);
							send(c_fd, m4.c_str(), m4.size(), 0);
							this->_clients.insert(std::make_pair(c_fd, newClient));
							this->fds.push_back((pollfd){c_fd, POLLIN, 0});
						}
					}
				}
				else
				{
					char buffer[1024];
					bzero(buffer, 1024);
					int n = recv(this->fds[i].fd, buffer, sizeof(buffer), 0);
					std::cout<<CYAN<<"Read : "<<buffer<<RST;
					if (n <= 0)
					{
						std::cout << BOLDMAGENTA << "Client disconected : " << fds[i].fd << RST << std::endl;
						close(fds[i].fd);
						std::map<int, Client *>::iterator cpos = this->_clients.find(fds[i].fd);
						if (cpos != this->_clients.end())
						{
							delete cpos->second;
							this->_clients.erase(cpos);
						}
						fds.erase(fds.begin() + i);
						i--;
					}
					else
					{
						buffer[n] = '\0';
						std::string msg(buffer);
						if (msg.find("PONG") == 0)
						{
							std::cout<<GREEN<<"PONG from : "<<this->fds[i].fd<<RST<<std::endl;
							continue;
						}
						else if (msg.find("PING") == 0)
						{
							for (size_t j = 0; j < this->fds.size(); j++)
							{
								if (this->fds[j].fd != this->_serverfd)
								{
									std::string ping = "PONG :"+this->_name+"\r\n";
									send(this->fds[j].fd, ping.c_str(), ping.size(), 0);
									std::cout<<GREEN<<"PONG send to : "<<this->fds[j].fd<<RST<<std::endl;
								}
							}
							continue;
						}
						else if (msg.find("JOIN") == 0)
							channelJoin(this->fds[i].fd, msg);
						else if (msg.find("INVITE") == 0)
							channelInvite(this->fds[i].fd, msg);
						else if (msg.find("PRIVMSG") == 0)
							msgBroadcast(this->fds[i].fd, msg);
						else if (msg.find("TOPIC") == 0)
							channelTopic(this->fds[i].fd, msg);
						else if (msg.find("PART") == 0)
							channelPart(this->fds[i].fd, msg);
						else if (msg.find("KICK") == 0)
							channelKick(this->fds[i].fd, msg);
						else if (msg.find("MODE") == 0)
							channelMode(this->fds[i].fd, msg);
						else if (msg.find("QUIT") == 0)
							serverQuit(this->findClient(this->fds[i].fd));
					}
				}
			}
		}
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

void Server::channelJoin(int fd, std::string cmd)
{
	char		characters[4] = {'&', '#', '+', '!'};
	std::string ch_name = "";

	for (int i = 0; i < 4; i++)
	{
		char c = characters[i];
		if (cmd.find(c) != std::string::npos)
		{
			size_t chname_end = cmd.find('\n');
			if (chname_end == std::string::npos)
				chname_end = cmd.find('\r');
			if (chname_end != std::string::npos)
			{
				ch_name = cmd.substr(cmd.find(c), chname_end - 1 - cmd.find(c));
				break;
			}
		}
	}
	std::cout<<GREY<<"Channel Join"<<std::endl<<"Channel name : ["<<ch_name<<"]"<<RST<<std::endl;
	if (ch_name.size() != 0)
	{
		Channel*	ch = findChannel(ch_name);
		Client*		clt = findClient(fd);
		if (!clt)
			return ;
		if (!ch)
		{
			Channel *newCh = new Channel(ch_name, clt);
			this->_channels.insert(std::make_pair(ch_name, newCh));
			std::string wmsg = ":"+clt->getNickname()+"!"+clt->getNickname()+"@127.0.0.1 JOIN "+ch_name+"\r\n";
			send(fd, wmsg.c_str(), wmsg.size(), 0);
			newCh->sendToAll(":"+clt->getNickname()+"!"+clt->getNickname()+"@127.0.0.1 JOIN :"+ch_name+"\r\n", this->_serverfd, fd);
			std::cout<<GREY<<"Channel Creation"<<std::endl<<"Client information : fd = "<<fd<<" - name : "<<clt->getUsername()<<RST<<std::endl;
		}
		else
			ch->joinChannel(clt, this->_serverfd, this->_name);
	}
}

void Server::sendMSG(int fd, std::string code, std::string uname, std::string spm, std::string tpm) const
{
	std::string msg = ":"+this->_name+" "+code+" "+uname+" "+spm+" :"+tpm+"\r\n";
	send(fd, msg.c_str(), msg.size(), 0);
	std::cout<<BOLDWHITE<<"SERVER : Message send to client "<<fd<<" : "<<msg<<RST<<std::endl;
}

void Server::msgBroadcast(int fd, std::string msg)
{
	size_t s_dp = msg.find(":");
	size_t e_msg = msg.find("\r");
	if (e_msg == std::string::npos)
		e_msg = msg.find("\n");
	std::string chname = msg.substr(8, (s_dp - 1) - 8);
	Channel	*ch = findChannel(chname);
	Client	*clt_rec = findClient(chname);
	if (ch && e_msg != std::string::npos)
	{
		Client*	clt = findClient(fd);
		if (!clt)
			return ;
		if (!ch->isInChannel(clt))
		{
			sendMSG(fd, "442", clt->getNickname(), chname, "You're not on that channel");
            return;
		}
		std::string tosend = ":"+clt->getNickname()+"!"+clt->getNickname()+"@127.0.0.1 PRIVMSG "+chname+" :"+msg.substr(s_dp + 1, e_msg)+"\r\n";
		ch->sendToAll(tosend, this->_serverfd, fd);
	}
	else if (clt_rec && e_msg != std::string::npos)
	{
		Client *clt_send = findClient(fd);
		if (clt_send)
		{
			std::string tosend = ":"+clt_send->getNickname()+"!"+clt_rec->getNickname()+"@127.0.0.1 PRIVMSG "+chname+" :"+msg.substr(s_dp + 1, e_msg)+"\r\n";
			int n = send(clt_rec->getFd(), tosend.c_str(), tosend.size(), 0);
			std::cout<<YELLOW<<"Private Message : "<<clt_send->getUsername()<<" to "<<clt_rec->getUsername()<<RST<<std::endl;
			std::cout<<YELLOW<<"Private Data : "<<n<<tosend<<RST<<std::endl;
		}
	}
}

const std::string Server::getName() const
{
	return this->_name;
}

void Server::channelPart(int fd, std::string cmd)
{
	std::cout<<BOLDYELLOW<<"channelPart called"<<RST<<std::endl;
	std::string ch_name = "";

	size_t part_pos = cmd.find("PART ");
	if (part_pos != std::string::npos)
	{
		size_t chname_start = part_pos + 5;
		size_t chname_end = cmd.find('\n', chname_start);
		if (chname_end == std::string::npos)
			chname_end = cmd.find('\r', chname_start);
		if (chname_end != std::string::npos)
		{
			ch_name = cmd.substr(chname_start, chname_end - chname_start);
			size_t end = ch_name.find_last_not_of(" \t\r\n");
			if (end != std::string::npos)
				ch_name = ch_name.substr(0, end + 1);
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
			sendMSG(fd, "403", clt->getNickname(), ch_name, "No such channel");
			return ;
		}
		if (!ch->isInChannel(clt))
		{
			sendMSG(fd, "442", clt->getNickname(), ch_name, "You're not on that channel");
			return ;
		}
		ch->leaveChannel(clt, this->_name, this->_serverfd);
	}
	else
	{
		Client *clt = findClient(fd);
        if (clt)
            sendMSG(fd, "461", clt->getNickname(), "PART", "Not enough parameters");
	}
}

void Server::channelKick(int fd, std::string cmd)
{
	char		characters[4] = {'&', '#', '+', '!'};
	std::string ch_name = "";
	std::string cltk_name = "";

	for (int i = 0; i < 4; i++)
	{
		char c = characters[i];
		if (cmd.find(c) != std::string::npos)
		{
			size_t chname_end = cmd.find('\n');
			if (chname_end == std::string::npos)
				chname_end = cmd.find('\r');
			if (chname_end != std::string::npos)
			{
				int ch_start = cmd.find(c);
				int ch_end = cmd.find(" ", ch_start);
				ch_name = cmd.substr(ch_start, ch_end - ch_start);
				int n_start = ch_end + 1;
				int n_end = cmd.find(" ", n_start);
				cltk_name = cmd.substr(n_start, n_end - n_start);
				break;
			}
		}
	}
	if (ch_name.size() > 0 && cltk_name.size() > 0)
	{
		Channel *chn = this->findChannel(ch_name);
		Client *clt_op = this->findClient(fd);
		Client *clt_tk = this->findClient(cltk_name);
		if (chn && clt_op && clt_tk)
			chn->kickChannel(clt_op, clt_tk, "parce que", this->_name);
	}
}

void Server::channelInvite(int fd, std::string cmd)
{
	char		characters[4] = {'&', '#', '+', '!'};
	std::string ch_name = "";
	std::string clti_name = "";

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
	std::cout<<BOLDMAGENTA<<"DEBUG INVITE :\nClient to invite :["<<clti_name<<"]\nOn Channel : ["<<ch_name<<"]"<<RST<<std::endl; 
	if (ch_name.size() > 0 && clti_name.size() > 0)
	{
		Channel *chn = this->findChannel(ch_name);
		Client *clt_snd = this->findClient(fd);
		Client *clt_rec = this->findClient(clti_name);
		if (!chn)
			std::cout<<BOLDRED<<"Y A PAS DE CHANNEL"<<RST<<std::endl;
		if (!clt_snd)
			std::cout<<BOLDRED<<"Y A PAS DE SENDER"<<RST<<std::endl;
		if (!clt_rec)
			std::cout<<BOLDRED<<"Y A PAS DE REC"<<RST<<std::endl;
		if (chn && clt_snd && clt_rec)
			chn->inviteChannel(clt_snd, clt_rec, this->_name);
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
					ntopic = cmd.substr(ch_end + 1, m_end - 1);
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
		ch->sndTopicChannel(clt, this->_name);
	else if (clt && ch && btopic)
		ch->setTopicChannel(clt, ntopic, this->_name);
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
				if (cend != std::string::npos)
				{
					ch_name = cmd.substr(cst, (cend - cst));
				}
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
	(void)fd;
	std::cout<<GREY<<"MODE DEBUG\n"<<"CHANNEL NAME : "<<ch_name<<"\nMODE : "<<mode_ch<<"\nTPARAM : "<<tparam<<RST<<std::endl;
}

void Server::clearClientChannel(Client *clt)
{
	for (std::map<std::string, Channel *>::iterator i; i != this->_channels.end(); i++)
	{
		if (i->second->isInChannel(clt))
			i->second->leaveChannel(clt, this->_name, this->_serverfd);
	}
}

void Server::serverQuit(Client *clt)
{
	if (clt == NULL)
		return ;
	for (std::map<std::string, Channel *>::iterator i = this->_channels.begin(); i != this->_channels.end(); i++)
	{
		if ((*i).second->isInChannel(clt))
			(*i).second->leaveChannel(clt, this->_name, this->_serverfd);
	}
	this->_clients.erase(clt->getFd());
}
