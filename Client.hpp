/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:51 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/11 17:14:13 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include "color.hpp"

class Client
{
	private:
		int			_fd;
		std::string _username;
		std::string _nickname;
		bool		_authenticated;
		//int		_ip;
	public:
		Client(int fd, std::string msg, std::string spass);
		Client(int fd, std::string uname, std::string nname, bool auth);
		~Client();

		int					getFd() const;
		const	std::string getUsername() const;
		const	std::string getNickname() const;
		void	setAuth(bool auth);
		bool	isAuth() const;
};