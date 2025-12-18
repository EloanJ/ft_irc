/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:56 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/17 17:08:32 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Client.hpp"
#include <sys/socket.h>
#include <vector>
#include <map>

class Channel
{
	private:
		std::string	_name;
		bool					_invite;

		//bool					_btopic;
		//std::string				_stopic;

		//bool					_blimit;
		//int						_nlimit;

		//bool					_bkey;
		//std::string				_skey;

		std::vector<Client*>	_lclients;
		std::vector<Client*>	_operators;
	public:
		Channel(std::string name, Client* clt);
		void		sendToAll(std::string msg, int fd_server, int fd_sender);
		void		addToChannel(Client *clt);
		void		leaveChannel(Client *clt, std::string h_name, int fd_server);
		void		kickChannel(Client *clt_op, Client *clt_tk, std::string reason, std::string sname);
		void		inviteChannel(Client *clt_snd, Client *clt_rec, std::string sname);
		bool		isInChannel(Client *clt);
		bool		isOperator(Client *clt);
		std::string	getChName();
		~Channel();
};
