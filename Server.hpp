/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 14:26:27 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/17 17:10:18 by vduarte          ###   ########.fr       */
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
		int			startServer();
		void		channelJoin(int fd, std::string cmd);
		Channel		*findChannel(std::string name);
		Client		*findClient(int fd);
		Client		*findClient(std::string name);
		void		msgBroadcast(int fd, std::string msg);
		void		sendMSG(int fd, std::string code, std::string uname, std::string spm, std::string tpm) const;
		static void signalServer(int sig);
		const std::string	getName() const;
		void 		channelPart(int fd, std::string cmd);
		void		channelKick(int fd, std::string cmd);
		void		channelInvite(int fd, std::string cmd);
		void		serverQuit(Client *clt);
};
