/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 14:30:33 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/03 16:39:28 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port, std::string pswd) : _password(pswd)
{
	this->_serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_serverfd < 0)
		std::cout<<RED<<"socket error"<<RST<<std::endl;
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
 	bind(this->_serverfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	listen(this->_serverfd, -1);
	this->fds.push_back((pollfd){this->_serverfd, POLLIN, 0}); // fd, event, revents
}

int	Server::startServer()
{
	std::cout<<GREEN<<"Server starting..."<<RST<<std::endl;
	while (true)
	{
		std::cout<<"in loop"<<std::endl;
		if (poll(&this->fds[0], this->fds.size(), -1) , 0)
			std::cout<<RED<<"poll error"<<std::endl;
		std::cout<<this->fds.size()<<std::endl;
		for (size_t i = 0; i < this->fds.size(); i++)
		{
			if (this->fds[i].revents & POLLIN)
			{
				if (this->fds[i].fd == this->_serverfd)
				{
					int c_fd = accept(this->_serverfd, NULL, NULL);
					if (c_fd < 1)
						std::cout<<RED<<"accept error"<<RST<<std::endl;
					else
						std::cout<<YELLOW<<"Client: "<<c_fd<<" accepted"<<RST<<std::endl;
					this->fds.push_back((pollfd){c_fd, POLLIN, 0});
				}
				else
				{
					char buffer[1024];
					int n = recv(this->fds[i].fd, buffer, sizeof(buffer), 0);
					std::cout<<"Read : "<<n<<std::endl;
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
						std::cout<<"Client: "<<this->fds[i].fd
							<<" say : " <<buffer<<std::endl;
					}
				}
			}
		}
	}
}

Server::~Server() {}