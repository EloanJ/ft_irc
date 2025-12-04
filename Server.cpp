/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 14:30:33 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/04 17:33:50 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

static bool g_stopSignal = false;

void Server::signalServer(int sig)
{
	if (sig == SIGINT || sig == SIGQUIT)
	{
		g_stopSignal = 1;
		std::cout<<ORANGE<<"Trying to close the server"<<RST<<std::endl;
	}
}

Server::Server(char* port, std::string pswd) : _password(pswd)
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

Client *Server::createNewClient(std::string msg)
{
	std::cout<<BOLDYELLOW<<"INFO : "<<msg<<RST<<std::endl;
	size_t st_nname = msg.find("NICK");
	std::string nickname;
	if (st_nname != std::string::npos)
	{
		size_t end = msg.find('\r', st_nname);
		if (end == std::string::npos)
			end = msg.find('\n', st_nname);
		if (end != std::string::npos)
			nickname = msg.substr(st_nname + 5, end - (st_nname + 5));
	}
	size_t st_uname = msg.find(":");
	std::string username;
	if (st_uname != std::string::npos)
	{
		size_t end = msg.find('\r', st_uname);
		if (end == std::string::npos)
			end = msg.find('\n', st_uname);
		if (end != std::string::npos)
			username = msg.substr(st_uname + 1, end - (st_uname + 1));
	}
	std::cout<<BOLDWHITE<<"Nickname a partir de "<<st_nname<<" : "<<nickname<<RST<<std::endl;
	std::cout<<BOLDWHITE<<"Username a partir de "<<st_uname<<" : "<<username<<RST<<std::endl;
	Client* newClient = new Client("ip", username, nickname);
	return newClient;
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
		std::cout<<"in loop"<<std::endl;
		if (poll(&this->fds[0], this->fds.size(), -1) < 0)
			std::cerr<<RED<<"poll error"<<RST<<std::endl;
		time_t now = std::time(NULL);
		std::cout<<this->fds.size()<<std::endl;
		std::cout<<"time ping : "<<now - this->_lastPing<<std::endl;
		if (now - this->_lastPing >= 30)
		{
			for (size_t j = 0; j < this->fds.size(); j++)
			{
				if (this->fds[j].fd != this->_serverfd)
				{
					std::string ping = "PING :irc.serv\r\n";
					send(this->fds[j].fd, ping.c_str(), ping.size(), 0);
					std::cout<<"PING send to : "<<this->fds[j].fd<<std::endl;
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
						std::string clientInfoTotal;
						int end;
						//std::cout<<"START OF CLIENT LOOP"<<std::endl;
						while ((end = recv(c_fd, clientInfo, 1024, 0)) > 0)
						{
							clientInfo[end] = '\0';
							clientInfoTotal += clientInfo;							
							if (clientInfoTotal.find("USER", 0) != std::string::npos)
								break;
							//std::cout<<"CLIENT LOOP"<<std::endl<<"BUFFER : "<<clientInfo<<std::endl<<"TOTAL BUFFER : "<<clientInfoTotal<<std::endl;
						}
						//std::cout<<"END OF CLIENT LOOP"<<std::endl;
						Client* newClient = createNewClient(clientInfoTotal);
						if (newClient != NULL)
						{
							const std::string nname = newClient->getNickname();
							const std::string uname = newClient->getUsername();
							std::string m1 = ":irc42.serv 001 "+nname+" :Welcome to the Internet Relay Chat Network, "+nname+"! "+nname+"@host\r\n";
							std::string m2 = ":irc42.serv 002 "+nname+" :Your host is irc42.serv, running version 1.0\r\n";
							std::string m3 = ":irc42.serv 003 "+nname+" :This server was created Dec 4 2025\r\n";
							std::string m4 = ":irc42.serv 004 "+nname+" irc42.serv 1.0 io i\r\n";
							std::cout<<YELLOW<<"Client: "<<newClient->getUsername()<<" accepted"<<RST<<std::endl;
							send(c_fd, m1.c_str(), m1.size(), 0);
							send(c_fd, m2.c_str(), m2.size(), 0);
							send(c_fd, m3.c_str(), m3.size(), 0);
							send(c_fd, m4.c_str(), m4.size(), 0);
							this->_clients.push_back(newClient);
						}
					}
					this->fds.push_back((pollfd){c_fd, POLLIN, 0});
				}
				else
				{
					char buffer[1024];
					int n = recv(this->fds[i].fd, buffer, sizeof(buffer), 0);
					std::cout<<"Read : "<<buffer;
					if (n <= 0)
					{
						std::cout << BOLDMAGENTA << "Client disconected : " << fds[i].fd << RST << std::endl;
						close(fds[i].fd);
						fds.erase(fds.begin() + 1);
						i--;
					}
					else
					{
						buffer[n] = '\0';
						std::string msg(buffer);
						if (msg.rfind("PONG :myserver\r\n", 0) == 0)
						{
							std::cout<<"PONG from : "<<this->fds[i].fd<<std::endl;
							continue;
						}
						else if (msg.rfind("PING :irc42.serv\r\n") == 0)
						{
							for (size_t j = 0; j < this->fds.size(); j++)
							{
								if (this->fds[j].fd != this->_serverfd)
								{
									std::string ping = "PONG :irc.serv\r\n";
									send(this->fds[j].fd, ping.c_str(), ping.size(), 0);
									std::cout<<"PONG send to : "<<this->fds[j].fd<<std::endl;
								}
							}
							continue;
						}
						else if (msg.find("JOIN") == 0)
							this->sendError(this->fds[i].fd, "403", "vduarte","#general", "No such channel");
					}
				}
			}
		}
	}
}

void Server::sendError(int fd, std::string code, std::string fpm, std::string spm, std::string tpm) const
{
	std::string error = ":irc42.serv "+code+" "+fpm+" "+spm+" :"+tpm+"\r\n";
	send(fd, error.c_str(), error.size(), 0);
	std::cerr<<BOLDRED<<"SERVER : Error send to client "<<fd<<" : "<<error<<RST<<std::endl;
}

Server::~Server() {}