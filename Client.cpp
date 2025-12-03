/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:53 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/03 13:17:52 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(std::string ip, std::string uname, std::string nname) : _ip(ip), _username(uname), _nickname(nname) {}

Client::~Client() {}