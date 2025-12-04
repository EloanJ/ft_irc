/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:44:37 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/04 12:25:13 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "color.hpp"
#include "Server.hpp"

int main(int ac, char **av)
{
	(void)av;
	if (ac != 3)
		std::cout<<RED<<"Please use this program correctly : ./ircserv port password"<<RST<<std::endl;
	else
	{
		std::cout<<GREEN<<"Thanks to use our service !"<<RST<<std::endl;
		Server _srv(av[1], "salut");
		_srv.startServer();
	}
}
