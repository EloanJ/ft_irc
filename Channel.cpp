/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:59 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/11 18:38:59 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(std::string name, Client* clt)
{
	this->_name = name;
	if (clt != NULL)
	{
		this->_lclients.push_back(clt);
		this->_operators.push_back(clt);
	}
}

Channel::~Channel() {}

void Channel::sendToAll(std::string msg, int fd_server, int fd_sender)
{
	std::cout<<BOLDYELLOW<<"lclients : "<<&this->_lclients<<RST<<std::endl;
	std::cout<<BOLDYELLOW<<"lclient size : "<<this->_lclients.size()<<std::endl;
	if (this->_lclients.size() != 0)
	{
		std::cout<<"je passe le if"<<std::endl;
		for (std::vector<Client *>::iterator i = this->_lclients.begin(); i != this->_lclients.end(); i++)
		{
			std::cout<<"boucle send all"<<std::endl;
			if (*i == NULL)
				continue;
			if ((*i)->getFd() != fd_server && (*i)->getFd() != fd_sender)
			send((*i)->getFd(), msg.c_str(), msg.size(), 0);
		}
	}
}

void Channel::addToChannel(Client *clt)
{
	this->_lclients.push_back(clt);
}