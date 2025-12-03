/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:46 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/03 13:08:57 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include "Client.hpp"

class Message
{
	private:
		std::string _message;
		Client &_client;
	public:
		Message(std::string msg, Client &clt);
		~Message();
};
