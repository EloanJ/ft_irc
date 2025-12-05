/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:56:56 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/05 12:02:56 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"
#include "Client.hpp"
#include <vector>
#include <map>

class Channel
{
	private:
		std::string	_name;
		std::string	_topic;
		std::string	_password;
		//char		_mode;
		//int			_userlimit;
		std::vector<Client*> _clients;
	public:
		Channel(std::string name, std::string topic, std::string password);
		~Channel();
};
