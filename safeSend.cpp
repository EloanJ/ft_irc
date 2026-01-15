/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   safeSend.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 12:21:38 by vduarte           #+#    #+#             */
/*   Updated: 2026/01/14 13:42:02 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "safeSend.hpp"

ssize_t safeSend(int fd, std::string ts)
{
	ssize_t res = send(fd, ts.c_str(), ts.size(), MSG_NOSIGNAL);
	if (res != static_cast<ssize_t>(ts.size()))
		std::cerr<<BOLDRED<<"SEND : Error on send size"<<RST<<std::endl;
	return res;
}
