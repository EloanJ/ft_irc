/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:51 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/03 13:15:01 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>

class Client
{
	private:
		std::string _ip;
		std::string _username;
		std::string _nickname;
	public:
		Client(std::string ip, std::string uname, std::string nnname);
		~Client();
};