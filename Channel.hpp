/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejonsery <ejonsery@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:56 by vduarte           #+#    #+#             */
/*   Updated: 2026/01/13 15:47:08 by ejonsery         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Client.hpp"
#include <sys/socket.h>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>

class Channel
{
	private:
		std::string	_name;
		bool					_btopic;
		bool					_botopic;
		std::string				_stopic;
		bool					_invite;
		bool					_blimit;
		size_t					_nlimit;
		bool					_bkey;
		std::string				_skey;
		std::string *_sname;
		std::vector<Client*>	_lclients;
		std::vector<Client*>	_operators;
		std::vector<Client*>	_cinvites;
	public:
		Channel(std::string name, Client* clt, std::string *sname);
		~Channel();
		void		sendToAll(std::string msg, int fd_sender);
		void		addToChannel(Client *clt);
		void		joinChannel(Client *clt, std::string key);
		void		leaveChannel(Client *clt, std::string reason);
		void		kickChannel(Client *clt_op, Client *clt_tk, std::string reason);
		void		inviteChannel(Client *clt_snd, Client *clt_rec);
		void		modeChannel(Client *clt, std::string md, std::string tp);
		void		modeAdd(Client *clt, std::string md, std::string tp);
		void		modeDrop(Client *clt, std::string md, std::string tp);
		void		sndModeChannel(Client *clt, bool sndAll);
		void		sndTopicChannel(Client *clt);
		void		sndTopicAll();
		void		sndLClients(Client *clt);
		void		setTopicChannel(Client *clt, std::string nname);
		bool		isInChannel(Client *clt);
		bool		isInvitetoChannel(Client *clt);
		bool		isOperator(Client *clt);
		Client		*isInChannel(std::string nname);
		std::string	getChName();
};
