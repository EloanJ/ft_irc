/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejonsery <ejonsery@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:53 by vduarte           #+#    #+#             */
/*   Updated: 2026/01/09 10:37:56 by ejonsery         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd)
{
	std::cout<<GREY<<"User creation\n"<<RST<<std::endl;
	this->_fd = fd;
	this->_createStep = 0;
	this->_nickname = "";
	this->_username = "";
	this->_sevname = "";
	this->_authenticated = false;
}

void Client::setPassword(std::string cmd, std::string pass)
{
	if (this->_createStep != 0)
		return ;
	std::string password = "";
	size_t st_pass = cmd.find(' ');
	if (st_pass == std::string::npos)
		st_pass = cmd.find('\r');
	if (st_pass == std::string::npos)
		st_pass = cmd.find('\n');
	if (st_pass != std::string::npos)
	{
		size_t end_pass = cmd.find('\r', (st_pass + 1));
		if (end_pass == std::string::npos)
			end_pass = cmd.find('\n', (st_pass + 1));
		if (end_pass != std::string::npos)
			password = cmd.substr((st_pass + 1), (end_pass - (st_pass + 1)));
		else
			password = cmd.substr((st_pass + 1), (cmd.size() - (st_pass + 1)));
	}
	this->_authenticated = password == pass;
	this->_createStep++;
	if (!this->_authenticated)
		this->_createStep = -1;
	std::cout<<GREY<<"Client password : ["<<password<<"]"<<RST<<std::endl;
}

void Client::setNickname(std::string cmd)
{
	if (this->_createStep != 1)
		return ;
	size_t st_nname = cmd.find(' ');
	if (st_nname == std::string::npos)
		st_nname = cmd.find('\r');
	if (st_nname == std::string::npos)
		st_nname = cmd.find('\n');
	if (st_nname != std::string::npos)
	{
		size_t end_nname = cmd.find('\r', (st_nname + 1));
		if (end_nname == std::string::npos)
			end_nname = cmd.find('\n', (st_nname + 1));
		if (end_nname != std::string::npos)
			this->_nickname = cmd.substr((st_nname + 1), (end_nname - (st_nname + 1)));
	}
	this->_createStep++;
	std::cout<<GREY<<"Client nickname : ["<<_nickname<<"]"<<RST<<std::endl;
}

void Client::setUsernameServername(std::string cmd)
{
	if (this->_createStep != 2)
		return ;
	size_t st_uname = cmd.find(':');
	if (st_uname == std::string::npos)
		st_uname = cmd.find('\r');
	if (st_uname == std::string::npos)
		st_uname = cmd.find('\n');
	if (st_uname != std::string::npos)
	{
		size_t end_uname = cmd.find('\r', (st_uname + 1));
		if (end_uname == std::string::npos)
			end_uname = cmd.find('\n', (st_uname + 1));
		if (end_uname != std::string::npos)
			this->_username = cmd.substr((st_uname + 1), (end_uname - (st_uname + 1)));
	}
	size_t end_sn = st_uname - 2;
	size_t st_sn = cmd.rfind(' ', end_sn);
	if (st_sn != std::string::npos)
		this->_sevname = cmd.substr(st_sn + 1, end_sn - st_sn);
	this->_createStep++;
	std::cout<<GREY<<"Client username : ["<<_username<<"]"<<std::endl;
	std::cout<<GREY<<"Client servername : ["<<_sevname<<"]"<<RST<<std::endl;	
}


const std::string Client::getUsername() const
{
	return this->_username;
}

const std::string Client::getNickname() const
{
	return this->_nickname;
}

const std::string Client::getSevname() const
{
	return this->_sevname;
}

int Client::getCreateStep() const
{
	return this->_createStep;
}

int Client::getFd() const
{
	return this->_fd;
}

void Client::setCreateStep(int step)
{
	this->_createStep = step;
}

bool Client::isAuth() const
{
	return this->_authenticated;
}

void Client::setAuth(bool auth)
{
	this->_authenticated = auth;
}

Client::~Client() {}