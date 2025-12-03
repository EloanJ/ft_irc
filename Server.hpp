/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 14:26:27 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/03 16:14:07 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <vector>
//#include "Channel.hpp"
#include "Client.hpp"
#include "color.hpp"


class Server
{
	private:
		std::string				_password;
		//std::vector<Channel&>	_channels;
		//std::vector<Client&>	_clients;
		int					 	_serverfd;
		std::vector<pollfd>		fds;
	public:
		Server(int port, std::string pswd);
		~Server();
		int startServer();
};
