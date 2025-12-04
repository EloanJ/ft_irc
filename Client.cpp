/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:53 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/04 16:53:07 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(std::string ip, std::string uname, std::string nname) : _ip(ip), _username(uname), _nickname(nname) {}

const std::string Client::getUsername() const
{
	return this->_username;
}

const std::string Client::getNickname() const
{
	return this->_nickname;
}

const std::string Client::getIP() const
{
	return this->_ip;
}

Client::~Client() {}