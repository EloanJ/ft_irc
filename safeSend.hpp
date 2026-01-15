/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   safeSend.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 12:21:48 by vduarte           #+#    #+#             */
/*   Updated: 2026/01/14 12:23:07 by vduarte          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <sys/socket.h>
#include "color.hpp"

ssize_t safeSend(int fd, std::string ts);