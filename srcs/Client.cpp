/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:50:30 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/12 14:58:41 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"
#include "../includes/Utils.hpp"

Client::Client(void) : _fd(-1), _ip(""), _nickname("*"), _user("*"), _pass(""), _temporaryNick("")
{
	std::cout << "Client created" << std::endl;
}

int Client::getFd( void )
{
	return _fd;
}

void Client::setFd( int fd )
{
	_fd = fd;
}

void Client::setIp( std::string ip )
{
	_ip = ip;
}
