/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejonsery <ejonsery@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 14:30:33 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/15 14:51:30 by ejonsery         ###   ########.fr       */
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

Server::~Server() {}

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
		std::cout<<ORANGE<<"in loop"<<RST<<std::endl;
		if (poll(&this->fds[0], this->fds.size(), -1) < 0)
			std::cerr<<RED<<"poll error"<<RST<<std::endl;
		time_t now = std::time(NULL);
		std::cout<<BOLDWHITE<<this->fds.size()<<RST<<std::endl;
		std::cout<<GREEN<<"time ping : "<<now - this->_lastPing<<RST<<std::endl;
		if (now - this->_lastPing >= 30)
		{
			for (size_t j = 0; j < this->fds.size(); j++)
			{
				if (this->fds[j].fd != this->_serverfd)
				{
					std::string ping = "PING :irc.serv\r\n";
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
						else if (msg.find("PRIVMSG") == 0)
							msgBroadcast(this->fds[i].fd, msg);
						else if (msg.find("PART") == 0)
							channelPart(this->fds[i].fd, msg);
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
	{
		std::cout<<"it.first :"<<it->first<<"ch_name :"<<name<<std::endl;
		return it->second;
	}
	return NULL;
}

Client *Server::findClient(int fd)
{
	std::map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it != this->_clients.end())
		return it->second;
	return NULL;
}

void Server::channelJoin(int fd, std::string cmd)
{
	char		characters[4] = {'&', '#', '+', '!'};
	std::string ch_name = "";
	(void)fd;

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
				ch_name = cmd.substr(cmd.find(c) + 1, chname_end - 1 - (cmd.find(c) + 1));
				break;
			}
		}
	}
	std::cout<<"Channel join ch name : "<<ch_name<<std::endl;
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
			std::cout<<fd<<clt->getNickname()<<ch_name<<std::endl;
			sendMSG(fd, "332", clt->getNickname(), ch_name, "Welcome to this channel !");
		}
		else
		{
			std::string wmsg = ":"+clt->getNickname()+"!"+clt->getNickname()+"@127.0.0.1 JOIN "+ch_name+"\r\n";
			send(fd, wmsg.c_str(), wmsg.size(), 0);
			ch->sendToAll(":"+clt->getNickname()+"!"+clt->getNickname()+"@127.0.0.1 JOIN :"+ch_name+"\r\n", this->_serverfd, fd);
			std::cout<<fd<<clt->getNickname()<<ch_name<<std::endl;
			sendMSG(fd, "332", clt->getNickname(), ch_name, "Welcome to this channel !");
			ch->addToChannel(clt);
			/* if il est pas sous invit only
				join ch;
			else
				sendMSG(); */
		}
	}
	// else renvoyer une erreur
// else renvoyer une erreur vers le client
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
		std::string tosend = ":"+clt->getNickname()+"!"+clt->getNickname()+"@127.0.0.1 PRIVMSG #"+chname+" :"+msg.substr(s_dp + 1, e_msg)+"\r\n";
		ch->sendToAll(tosend, this->_serverfd, fd);
	}
}

std::string Server::getName()
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