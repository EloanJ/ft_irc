/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 14:26:27 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/05 12:58:30 by vduarte          ###   ########.fr       */
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
#include <signal.h>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include "Channel.hpp"
#include "Client.hpp"
#include "color.hpp"

class Server
{
	private:
		std::string						_name;
		std::string						_password;
		std::map<std::string, Channel*>	_channels;
		std::map<int, Client *>			_clients;
		int					 			_serverfd;
		time_t							_lastPing;
		std::vector<pollfd>				fds;
	public:
		Server(char* port, std::string pswd, std::string name);
		~Server();
		int			verifInput(std::string port);
		Client		*createNewClient(std::string msg);
		int			startServer();
		void		channelJoin(int fd, std::string cmd);
		Channel		*findChannel(std::string name);
		void		sendMSG(int fd, std::string code, std::string uname, std::string spm, std::string tpm) const;
		static void signalServer(int sig);
};
