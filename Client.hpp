/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:51 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/04 16:52:10 by vduarte          ###   ########.fr       */
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
		bool		_authenticated;
	public:
		Client(std::string ip, std::string uname, std::string nnname, bool auth);
		~Client();

		const	std::string getIP() const;
		const	std::string getUsername() const;
		const	std::string getNickname() const;
		void	setAuth(bool auth);
		bool	isAuth() const;
};