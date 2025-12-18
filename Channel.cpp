/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:59 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/17 17:36:40 by vduarte          ###   ########.fr       */
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

bool Channel::isOperator(Client *clt)
{
	for (std::vector<Client *>::iterator it = this->_operators.begin(); it != this->_operators.end(); it++)
	{
		if ((*it) == clt)
			return true;
	}
	return false;
}

void Channel::kickChannel(Client *clt_op, Client *clt_tk, std::string reason, std::string sname)
{
	bool isop = this->isOperator(clt_op);
	if (isop)
	{
		std::string to_send = ":"+clt_op->getNickname()+"!"+clt_tk->getNickname()+"@127.0.0.1 KICK "+this->_name+" "+clt_tk->getNickname()+"\r\n";
		send(clt_tk->getFd(), to_send.c_str(), to_send.size(), 0);
		for (std::vector<Client *>::iterator it = this->_lclients.begin(); it != this->_lclients.end(); it++)
		{
			if (*it == clt_tk)
			{
				this->_lclients.erase(it);
				break;
			}
		}
		for (std::vector<Client *>::iterator it = this->_operators.begin(); it != this->_operators.end(); it++)
		{
			if (*it == clt_tk)
			{
				this->_operators.erase(it);
				break;
			}
		}
		std::cout<<BOLDCYAN<<"User "+clt_op->getNickname()+" kick "<<clt_tk->getNickname()<<" for "<<reason<<RST<<std::endl;
	}
	else
	{
		std::string to_send = ":"+sname+" 482 "+this->_name+" :You're not channel operator\r\n"; // not operator : ERR_CHANOPRIVSNEEDED
		send(clt_op->getFd(), to_send.c_str(), to_send.size(), 0);
	}
}

void Channel::inviteChannel(Client *clt_snd, Client *clt_rec, std::string sname)
{
	std::cout<<BOLDMAGENTA<<"Welcome the best channel invitation in this world\n"<<RST;
	if (this->isInChannel(clt_snd) && !this->isInChannel(clt_rec))
	{
		if (!this->_invite)
		{
			std::cout<<BOLDMAGENTA<<"clt_snd in channel | clt_rec not in channel\n"<<RST;
			std::string to_send = ":"+clt_snd->getNickname()+"!"+clt_rec->getNickname()+"@127.0.0.1 INVITE "+clt_rec->getNickname()+" "+this->_name+"\r\n";
			send(clt_rec->getFd(), to_send.c_str(), to_send.size(), 0);
		}
		else if (this->_invite && this->isOperator(clt_snd))
		{
			std::string to_send = ":"+clt_snd->getNickname()+"!"+clt_rec->getNickname()+"@127.0.0.1 INVITE "+clt_rec->getNickname()+" "+this->_name+"\r\n";
			send(clt_rec->getFd(), to_send.c_str(), to_send.size(), 0);
		}
		else
		{
			std::string to_send = ":"+sname+" 482 "+this->_name+" :You're not channel operator\r\n"; // not operator : ERR_CHANOPRIVSNEEDED
			send(clt_snd->getFd(), to_send.c_str(), to_send.size(), 0);
		}
	}
	else if (!this->isInChannel(clt_snd))
	{
		std::string to_send = ":"+sname+" 442 "+this->_name+" :You're not on that channel\r\n"; // not on channel : ERR_NOTONCHANNEL
		send(clt_snd->getFd(), to_send.c_str(), to_send.size(), 0);
	}
	else if (this->isInChannel(clt_rec))
	{
		std::string to_send = ":"+sname+" 443 "+clt_rec->getNickname()+" "+this->_name+" :is already on channel\r\n"; // client already on channel : ERR_USERONCHANNEL
		send(clt_snd->getFd(), to_send.c_str(), to_send.size(), 0);
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