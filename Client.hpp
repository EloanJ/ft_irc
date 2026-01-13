/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejonsery <ejonsery@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:51 by vduarte           #+#    #+#             */
/*   Updated: 2026/01/13 15:27:23 by ejonsery         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <list>
#include "color.hpp"

class Client
{
	private:
		int						_fd;
		std::string 			_username;
		std::string 			_nickname;
		bool					_authenticated;
		std::string				_sevname;
		int						_createStep;
		std::list<std::string>	_cmds;
	public:
		Client(int fd);
		~Client();

		void							setPassword(std::string cmd, std::string pass);
		void							setNickname(std::string cmd);
		void							setUsernameServername(std::string cmd);
		void							setCreateStep(int step);
		void							setAuth(bool auth);
		void							addCmd(std::string cmd);
		void							clearCmd();
		bool							isAuth() const;
		bool							completeCmd();
		int								getCreateStep() const;
		int								getFd() const;
		const std::string 				getUsername() const;
		const std::string 				getNickname() const;
		const std::string 				getSevname() const;
		std::list<std::string>			&getCmds();
};
