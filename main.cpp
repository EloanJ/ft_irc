/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:44:37 by vduarte           #+#    #+#             */
/*   Updated: 2025/12/03 16:26:24 by vduarte          ###   ########.fr       */
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
		Server _srv(atoi(av[1]), "salut");
		_srv.startServer();
	}
}

// int main()
// {
// 	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
// 	sockaddr_in serverAddress;
// 	serverAddress.sin_family = AF_INET;
// 	serverAddress.sin_port = htons(8080);
// 	serverAddress.sin_addr.s_addr = INADDR_ANY;
// 	bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
// 	listen(serverSocket, 5);
// 	int clientSocket =accept(serverSocket, NULL, NULL);
// 	char buffer[1024] = {0};
// 	pollfd fdpoll = {clientSocket, POLLIN, 0};
// 	while (true)
// 	{
// 		poll(&fdpoll, POLLIN, -1);
// 		recv(clientSocket, buffer, sizeof(buffer), 0);
// 		std::cout << "Message from client: " << buffer << std::endl;
// 	}
// 	close(serverSocket);
// }