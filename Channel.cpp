/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejonsery <ejonsery@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:59 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/15 12:10:46 by ejonsery         ###   ########.fr       */
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
	std::cout<<BOLDYELLOW<<"lclient size : "<<this->_lclients.size()<<std::endl;
	if (this->_lclients.size() != 0)
	{
		for (std::vector<Client *>::iterator i = this->_lclients.begin(); i != this->_lclients.end(); i++)
		{
			if (*i == NULL)
				continue;
			if ((*i)->getFd() != fd_server && (*i)->getFd() != fd_sender)
				send((*i)->getFd(), msg.c_str(), msg.size(), 0);
		}
	}
}

void Channel::leaveChannel(Client *clt, std::string h_name, int fd_server)
{
	std::string part_msg = ":" + clt->getNickname() + "!" + clt->getNickname() + "@" + h_name + " PART " + this->_name + "\r\n";
	std::cout<<"part msg : "<<part_msg<<std::endl;
	for (std::vector<Client *>::iterator i = this->_lclients.begin(); i != this->_lclients.end(); i++)
	{
		std::cout<<GREY<<"dans la boucle"<<RST<<std::endl;
		if (*i == NULL)
			continue;
		if (*i == clt)
		{
			std::cout<<BOLDMAGENTA<<"FIND USER"<<RST<<std::endl;
			std::cout << "Envoi PART au client fd=" << clt->getFd() << std::endl;
    		int ret = send(clt->getFd(), part_msg.c_str(), part_msg.size(), 0);
   			std::cout << "Résultat send: " << ret << std::endl;
			this->sendToAll(part_msg, fd_server, clt->getFd());
			this->_lclients.erase(i);
			for (std::vector<Client *>::iterator op = this->_operators.begin(); op != this->_operators.end(); op++)
			{
				if (*op == clt)
				{
					this->_operators.erase(op);
					break;
				}
			}
			break;
		}
	}
}

void Channel::addToChannel(Client *clt)
{
	this->_lclients.push_back(clt);
}

bool Channel::isInChannel(Client *clt)
{
	for (std::vector<Client *>::iterator i = this->_lclients.begin(); i != this->_lclients.end(); i++)
	{
		if (*i == clt)
			return true;
	}
	return false;
}

std::string	Channel::getChName()
{
	return this->_name;
}