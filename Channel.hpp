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
		//Selon noe on pourrait faire un autre vector de client mais ceux qui sont operator. Cela rend plus simple de savoir qui est OP sur tel ou tel chan.
		//Donc n'importe qui peut faire /join et cree un chan si il existe pas et en devient +o, si il est deja cree il le rejoint normal et si il est sur invit-> error.
		public:
		Channel(std::string name, std::string topic, std::string password);
		~Channel();
};
