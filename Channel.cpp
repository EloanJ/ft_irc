/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejonsery <ejonsery@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:59 by vduarte           #+#    #+#             */
/*   Updated: 2026/01/13 15:55:06 by ejonsery         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(std::string name, Client* clt, std::string *sname)
{
	this->_name = name;
	this->_sname = sname;
	this->_btopic = false;
	this->_botopic = false;
	this->_invite = false;
	this->_blimit = false;
	this->_bkey = false;
	if (clt != NULL)
		this->_operators.push_back(clt);
}

Channel::~Channel()
{
	if (this->_lclients.size() > 0)
		this->_lclients.clear();
	if (this->_operators.size() > 0)
		this->_operators.clear();
	if (this->_cinvites.size() > 0)
		this->_cinvites.clear();
}

void Channel::sendToAll(std::string msg, int fd_sender)
{
	if (this->_lclients.size() != 0)
	{
		for (std::vector<Client *>::iterator i = this->_lclients.begin(); i != this->_lclients.end(); i++)
		{
			if (*i == NULL)
				continue;
			if ((*i)->getFd() != fd_sender)
				send((*i)->getFd(), msg.c_str(), msg.size(), 0);
		}
	}
}

void Channel::joinChannel(Client *clt, std::string key)
{
	std::cout<<GREY<<"Join Channel -- Step 2\n"<<RST;
	std::string ts = "";
	if (this->isInChannel(clt))
	{
		std::cout<<GREY<<"Client already on channel\n"<<RST;
		ts = ":"+(*_sname)+" 443 "+clt->getNickname()+" "+this->_name+" :is already on channel";
		send(clt->getFd(), ts.c_str(), ts.size(), 0);
	}
	else if (this->_blimit && this->_lclients.size() == this->_nlimit)
	{
		std::cout<<GREY<<"Channel limit reached\n"<<RST;
		ts = ":"+(*_sname)+" 471 "+clt->getNickname()+" "+this->_name+" :Cannot join channel (+l)";
		send(clt->getFd(), ts.c_str(), ts.size(), 0);
	}
	else if (this->_invite && !this->isInvitetoChannel(clt))
	{
		std::cout<<GREY<<"Channel is in invite-only mode\n"<<RST;
		ts = ":"+(*_sname)+" 473 "+clt->getNickname()+" "+this->_name+" :Cannot join channel (+i)";
		send(clt->getFd(), ts.c_str(), ts.size(), 0);
	}
	else if (this->_bkey && this->_skey != key)
	{
		std::cout<<GREY<<"Incorrect channel password\n"<<RST;
		ts = ":"+(*_sname)+" 475 "+clt->getNickname()+" "+this->_name+" :Cannot join channel (+k)";
		send(clt->getFd(), ts.c_str(), ts.size(), 0);
	}
	else
	{
		this->_lclients.push_back(clt);
		this->sendToAll(":"+clt->getNickname()+"!"+clt->getNickname()+"@"+clt->getSevname()+" JOIN :"+this->_name+"\r\n", -1);
		if (this->_stopic.size() > 0)
			this->sndTopicChannel(clt);
		this->sndLClients(clt);
		std::cout<<GREY<<"Client : ["+clt->getNickname()+"] joined ["+this->_name+"]\n"<<RST;
	}
}

void Channel::leaveChannel(Client *clt, std::string reason)
{
	if (reason.size() == 0)
		reason = "Bye bye 👋";
	std::string part_msg = ":" + clt->getNickname() + "!" + clt->getNickname() + "@" + clt->getSevname() + " PART " + this->_name + " :" + reason + "\r\n";
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
			this->sendToAll(part_msg, clt->getFd());
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

void Channel::kickChannel(Client *clt_op, Client *clt_tk, std::string reason)
{
	bool isop = this->isOperator(clt_op);
	if (isop)
	{
		if (!this->isInChannel(clt_tk))
		{
			std::string to_send = ":"+(*_sname)+" 441 " +clt_tk->getNickname()+" "+this->_name+" :They aren't on that channel\r\n";
			send(clt_op->getFd(), to_send.c_str(), to_send.size(), 0);
			return ;
		}
		std::string ts = ":"+clt_op->getNickname()+"!"+clt_tk->getNickname()+"@"+clt_tk->getSevname()+" KICK "+this->_name+" "+clt_tk->getNickname()+" :"+reason+"\r\n";
		sendToAll(ts, -1);
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
		std::string to_send = ":"+(*_sname)+" 482 "+this->_name+" :You're not channel operator\r\n";
		send(clt_op->getFd(), to_send.c_str(), to_send.size(), 0);
	}
}

void Channel::inviteChannel(Client *clt_snd, Client *clt_rec)
{
	std::cout<<GREY<<"INVITE -- STEP 2\n"<<RST;
	if (this->isInChannel(clt_snd) && !this->isInChannel(clt_rec))
	{
		if (!this->_invite || (this->_invite && this->isOperator(clt_snd)))
		{
			std::cout<<GREY<<"clt_snd in channel | clt_rec not in channel"<<RST<<std::endl;
			std::string ts_rec = ":"+clt_snd->getNickname()+"!"+clt_rec->getNickname()+"@"+clt_rec->getSevname()+" INVITE "+clt_rec->getNickname()+" "+this->_name+"\r\n";
			std::string ts_snd = ":"+(*_sname)+" 341 "+clt_snd->getNickname()+" "+this->_name+" "+clt_rec->getNickname()+"\r\n";
			send(clt_rec->getFd(), ts_rec.c_str(), ts_rec.size(), 0);
			send(clt_snd->getFd(), ts_snd.c_str(), ts_snd.size(), 0);
		}
		else
		{
			std::cout<<GREY<<"clt_snd isn't a channel operator"<<RST<<std::endl;
			std::string to_send = ":"+(*_sname)+" 482 "+this->_name+" :You're not channel operator\r\n";
			send(clt_snd->getFd(), to_send.c_str(), to_send.size(), 0);
		}
	}
	else if (!this->isInChannel(clt_snd))
	{
		std::cout<<GREY<<"clt_snd not in channel"<<RST<<std::endl;
		std::string to_send = ":"+(*_sname)+" 442 "+this->_name+" :You're not on that channel\r\n";
		send(clt_snd->getFd(), to_send.c_str(), to_send.size(), 0);
	}
	else if (this->isInChannel(clt_rec))
	{
		std::cout<<GREY<<"clt_snd in channel | clt_rec in channel"<<RST<<std::endl;
		std::string to_send = ":"+(*_sname)+" 443 "+clt_rec->getNickname()+" "+this->_name+" :is already on channel\r\n";
		send(clt_snd->getFd(), to_send.c_str(), to_send.size(), 0);
	}
}

void Channel::modeChannel(Client *clt, std::string md, std::string tp)
{
	std::cout<<GREY<<"CHANNEL MODE"<<std::endl;
	std::string ts;
	if (clt)
	{
		std::cout<<"CLIENT FOUND"<<std::endl;
		if (md.size() == 0)
			this->sndModeChannel(clt, false);
		else if (this->isOperator(clt))
		{
			std::cout<<"CLIENT IS OPERATOR"<<std::endl;
			if (md.size() != 3) // limitation of 2 caracters par mode (+/-, itkol et \0)
			{
				std::cout<<"MODE IS CORRECT"<<std::endl;
				if (md.find('+') != std::string::npos && md.find('+') == 0)
					this->modeAdd(clt, md, tp);
				else if (md.find('-') != std::string::npos && md.find('-') == 0)
					this->modeDrop(clt, md, tp);
			}
			else
			{
				ts = ":"+(*_sname)+" 472 "+md.substr(1, (md.size() - 1))+" :is unknown mode char to me for "+this->_name+"\r\n";
				send(clt->getFd(), ts.c_str(), ts.size(), 0);
			}
		}
		else
		{
			ts = ":"+(*_sname)+" 482 "+this->_name+" :You're not channel operator\r\n";
			send(clt->getFd(), ts.c_str(), ts.size(), 0);
		}
	}
	std::cout<<RST;
}

void Channel::modeAdd(Client *clt, std::string md, std::string tp)
{
	std::string ts;
	std::cout<<"MODE IS +"<<std::endl;
	if (md[1] == 'i')
		this->_invite = true;
	else if (md[1] == 't')
		this->_botopic = true;
	else if (md[1] == 'k')
	{
		if (!this->_bkey && !this->_skey.size() && tp.size() > 0)
		{
			this->_bkey = true;
			this->_skey = tp;
		}
		else if (this->_bkey)
		{
			ts = ":"+(*_sname)+" 467 "+clt->getNickname()+" "+this->_name+" :Channel key already set\r\n";
			send(clt->getFd(), ts.c_str(), ts.size(), 0);
		}
		else if (tp.size() == 0)
		{
			ts = ":"+(*_sname)+" 461 "+clt->getNickname()+" +k :Not enough parameters\r\n";
			send(clt->getFd(), ts.c_str(), ts.size(), 0);
		}
	}
	else if (md[1] == 'o')
	{
		if (tp.size() == 0)
		{
			std::cout<<GREY<<"need more params +o"<<RST<<std::endl;
			ts = ":"+(*_sname)+" 461 "+clt->getNickname()+" +o :Not enough parameters\r\n";
			send(clt->getFd(), ts.c_str(), ts.size(), 0);							
		}
		else
		{
			Client *clt_add = this->isInChannel(tp);
			if (!clt_add)
			{
				ts = ":"+(*_sname)+" 441 "+tp+" "+this->_name+" :They aren't on that channel\r\n";
				send(clt->getFd(), ts.c_str(), ts.size(), 0);
			}
			if (clt_add && !this->isOperator(clt_add))
				this->_operators.push_back(clt_add);
		}
	}
	else if (md[1] == 'l')
	{
		if (!this->_blimit && tp.size() != 0)
		{
			int l = atoi(tp.c_str());
			if (l > 0)
			{
				this->_blimit = true;
				this->_nlimit = l;
			}
		}
		else if (tp.size() == 0)
		{
			ts = ":"+(*_sname)+" 461 "+clt->getNickname()+" +l :Not enough parameters\r\n";
			send(clt->getFd(), ts.c_str(), ts.size(), 0);
		}
	}
	else
	{
		ts = ":"+(*_sname)+" 472 "+md.substr(1, (md.size() - 1))+" :is unknown mode char to me for "+this->_name+"\r\n";
		std::cout<<BOLDMAGENTA<<ts<<std::endl<<RST;
		send(clt->getFd(), ts.c_str(), ts.size(), 0);
	}
	this->sndModeChannel(clt, true);
}

void Channel::modeDrop(Client *clt, std::string md, std::string tp)
{
	std::string ts;
	std::cout<<"MODE IS -"<<std::endl;
	if (md[1] == 'i')
		this->_invite = false;
	else if (md[1] == 't')
		this->_botopic = false;
	else if (md[1] == 'k')
	{
		if (this->_bkey && this->_skey.size() > 0 && tp == this->_skey)
		{
			this->_bkey = false;
			this->_skey.clear();
		}
		else if (tp.size() == 0)
		{
			std::cout<<GREY<<"need more params -k"<<RST<<std::endl;
			ts = ":"+(*_sname)+" 461 "+clt->getNickname()+" -k :Not enough parameters\r\n"; 
			send(clt->getFd(), ts.c_str(), ts.size(), 0);		
		}
	}
	else if (md[1] == 'o')
	{
		Client *clt_rm = this->isInChannel(tp);
		if (clt_rm && this->isOperator(clt_rm))
		{
			for (std::vector<Client *>::iterator i = this->_operators.begin(); i != this->_operators.end(); i++)
			{
				if ((*i) == clt_rm)
				{
					this->_operators.erase(i);
					break;
				}
			}
		}
	}
	else if (md[1] == 'l')
	{
		if (this->_blimit)
		{
			this->_blimit = false;
			this->_nlimit = 0;
		}
	}
	else
	{
		ts = ":"+(*_sname)+" 472 "+md.substr(1, (md.size() - 1))+" :is unknown mode char to me for "+this->_name+"\r\n";
		send(clt->getFd(), ts.c_str(), ts.size(), 0);
	}
	this->sndModeChannel(clt, true);
}

void Channel::sndModeChannel(Client *clt, bool sndAll)
{
	bool first = true;
	std::string ts = ":"+(*_sname)+" 324 "+clt->getNickname()+" "+this->_name;
	if (this->_invite)
	{
		if (first)
			ts += " +i";
		else
			ts += "i";
		first = false;
	}
	if (this->_botopic)
	{
		if (first)
			ts += " +t";
		else
			ts += "t";
		first = false;
	}
	if (this->_bkey)
	{
		if (first)
			ts += " +k";
		else
			ts += "k";
		first = false;
	}
	if (this->_blimit)
	{
		if (first)
			ts += " +l";
		else
			ts += "l";
		first = false;
	}
	if (this->_bkey)
		ts += " "+this->_skey;
	if (this->_blimit)
	{
		std::stringstream ss;
		ss << this->_nlimit;
		ts += " "+ss.str();
	}
	ts += "\r\n";
	std::cout<<"mode send : ["<<ts<<"]"<<std::endl;
	if (sndAll)
		this->sendToAll(ts, -1);
	else
		send(clt->getFd(), ts.c_str(), ts.size(), 0);
}

void Channel::sndTopicChannel(Client *clt)
{
	std::cout<<GREY<<"DEBUG OF SENDTOPIC\n"<<RST;
	std::string to_send = "";
	if (this->_btopic)
		to_send = ":"+(*_sname)+" 332 "+clt->getNickname()+" "+this->_name+" :"+this->_stopic+"\r\n";
	else if (!this->_btopic)
		to_send = ":"+(*_sname)+" 331 "+clt->getNickname()+" "+this->_name+" :No topic is set\r\n";
	std::cout<<GREY<<"TO SEND : "+to_send+"\n"<<RST;
	send(clt->getFd(), to_send.c_str(), to_send.size(), 0);
}

void Channel::sndLClients(Client *clt)
{
	std::string ts = ":"+(*_sname)+" 353 "+clt->getNickname()+" = "+this->_name+" :";
	for (std::vector<Client *>::const_iterator it = this->_lclients.begin(); it != this->_lclients.end(); it ++)
	{
		if (this->isOperator((*it)))
			ts += "@"+(*it)->getNickname();
		else
			ts += (*it)->getNickname();
		if (it + 1 != this->_lclients.end())
			ts += " ";
	}
	ts += "\r\n";
	std::cout<<GREY<<"user lst ["<<ts<<"]"<<RST<<std::endl;
	send(clt->getFd(), ts.c_str(), ts.size(), 0);
	ts = ":"+(*_sname)+" 366 "+clt->getNickname()+" "+this->_name+" :End of NAMES list\r\n";
	send(clt->getFd(), ts.c_str(), ts.size(), 0);
}

void Channel::sndTopicAll()
{
	std::string ts = "";
	for (std::vector<Client *>::iterator it = this->_lclients.begin(); it != this->_lclients.end(); it ++)
	{
		if ((*it) != NULL)
		{
			ts = ":"+(*_sname)+" 332 "+(*it)->getNickname()+" "+this->_name+" :"+this->_stopic+"\r\n";
			send((*it)->getFd(), ts.c_str(), ts.size(), 0);
		}
	}
}

void Channel::setTopicChannel(Client *clt, std::string nname)
{
	if ((this->isInChannel(clt) && this->isOperator(clt)) || (this->isInChannel(clt) && !this->_botopic))
	{
		if (nname.size() == 0)
		{
			this->_stopic = nname;
			this->_btopic = false;
		}
		else
		{
			this->_stopic = nname;
			this->_btopic = true;
		}
		this->sndTopicAll();
	}
	else if (this->isInChannel(clt) && !this->isOperator(clt))
	{
		std::string to_send = ":"+(*_sname)+" 482 "+this->_name+" :You're not channel operator\r\n";
		send(clt->getFd(), to_send.c_str(), to_send.size(), 0);
	}
	else if (!this->isInChannel(clt))
	{
		std::string to_send = ":"+(*_sname)+" 442 "+this->_name+" :You're not on that channel\r\n";
		send(clt->getFd(), to_send.c_str(), to_send.size(), 0);
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

Client *Channel::isInChannel(std::string nname)
{
	for (std::vector<Client *>::iterator it = this->_lclients.begin(); it !=this->_lclients.end(); it++)
	{
		if ((*it)->getNickname() == nname)
			return (*it);
	}
	return NULL;
}

bool Channel::isInvitetoChannel(Client *clt)
{
	for (std::vector<Client *>::iterator i = this->_cinvites.begin(); i != this->_cinvites.end(); i++)
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