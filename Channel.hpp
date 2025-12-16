/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejonsery <ejonsery@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:56 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/15 11:10:26 by ejonsery         ###   ########.fr       */
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
		std::string	_topic;
		//faut faire les modes;
		//int			_userlimit;
		std::vector<Client*> _lclients;
		std::vector<Client*> _operators;
		//Selon noe on pourrait faire un autre vector de client mais ceux qui sont operator. Cela rend plus simple de savoir qui est OP sur tel ou tel chan.
		//Donc n'importe qui peut faire /join et cree un chan si il existe pas et en devient +o, si il est deja cree il le rejoint normal et si il est sur invit-> error.
	public:
		Channel(std::string name, Client* clt);
		void		sendToAll(std::string msg, int fd_server, int fd_sender);
		void		addToChannel(Client *clt);
		void		leaveChannel(Client *clt, std::string h_name, int fd_server);
		bool		isInChannel(Client *clt);
		std::string	getChName();
		~Channel();
};
