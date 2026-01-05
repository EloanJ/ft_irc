/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:56 by vduarte           #+#    #+#             */
/*   Updated: 2026/01/05 16:14:59 by vduarte          ###   ########.fr       */
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
		bool					_btopic;
		std::string				_stopic;

		bool					_invite;

		bool					_blimit;
		size_t					_nlimit;

		bool					_bkey;
		//std::string				_skey;

		std::vector<Client*>	_lclients;
		std::vector<Client*>	_operators;
		std::vector<Client*>	_cinvites;
	public:
		Channel(std::string name, Client* clt);
		void		sendToAll(std::string msg, int fd_server, int fd_sender);
		void		addToChannel(Client *clt);
		void		joinChannel(Client *clt, int sfd, std::string sname);
		void		leaveChannel(Client *clt, std::string h_name, int fd_server);
		void		kickChannel(Client *clt_op, Client *clt_tk, std::string reason, std::string sname);
		void		inviteChannel(Client *clt_snd, Client *clt_rec, std::string sname);
		void		modeChannel(Client *clt, std::string sname);
		void		sndTopicChannel(Client *clt, std::string sname);
		void		setTopicChannel(Client *clt, std::string nname, std::string sname);
		bool		isInChannel(Client *clt);
		bool		isInvitetoChannel(Client *clt);
		bool		isOperator(Client *clt);
		std::string	getChName();
		~Channel();
};
