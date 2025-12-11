/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:53 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/11 18:38:34 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd, std::string uname, std::string nname, bool auth) : _fd(fd), _username(uname), _nickname(nname), _authenticated(auth) {}

Client::Client(int fd, std::string msg, std::string spass)
{
	std::cout<<BOLDYELLOW<<"INFO : "<<msg<<RST<<std::endl;
	size_t st_pass = msg.find("PASS");
	std::string password;
	if (st_pass != std::string::npos)
	{
		size_t end = msg.find('\r', st_pass + 5);
		if (end == std::string::npos)
			end = msg.find('\n', st_pass + 5);
		if (end != std::string::npos)
			password = msg.substr(st_pass + 5, end - (st_pass + 5));
	}
	size_t st_nname = msg.find("NICK");
	std::string nickname;
	if (st_nname != std::string::npos)
	{
		size_t end = msg.find('\r', st_nname);
		if (end == std::string::npos)
			end = msg.find('\n', st_nname);
		if (end != std::string::npos)
			nickname = msg.substr(st_nname + 5, end - (st_nname + 5));
	}
	size_t st_uname = msg.find(":", msg.find("USER"));
	std::string username;
	if (st_uname != std::string::npos)
	{
		size_t end = msg.find('\r', st_uname);
		if (end == std::string::npos)
			end = msg.find('\n', st_uname);
		if (end != std::string::npos)
			username = msg.substr(st_uname + 1, end - (st_uname + 1));
	}
	/* Je suis un find de l'ip */
	std::cout<<BOLDWHITE<<"Password a partir de "<<st_pass<<" : "<<password<<RST<<std::endl;
	std::cout<<BOLDWHITE<<"Nickname a partir de "<<st_nname<<" : "<<nickname<<RST<<std::endl;
	std::cout<<BOLDWHITE<<"Username a partir de "<<st_uname<<" : "<<username<<RST<<std::endl;
	this->_fd = fd;
	this->_nickname = nickname;
	this->_username = username;
	this->_authenticated = password == spass;
}

const std::string Client::getUsername() const
{
	return this->_username;
}

const std::string Client::getNickname() const
{
	return this->_nickname;
}

int Client::getFd() const
{
	return this->_fd;
}

void Client::setAuth(bool auth)
{
	this->_authenticated = auth;
}

bool Client::isAuth() const
{
	return this->_authenticated;
}

Client::~Client() {}