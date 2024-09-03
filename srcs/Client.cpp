/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 11:36:09 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/03 13:04:06 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"
#include "../includes/Utils.hpp"

Client::Client(void) : _fd(-1), _ip(""), _nickname(""), _user(""), _pass("")
{
	std::cout << PUR << "Client created" << WHI << std::endl;
}

int Client::getFd(void)
{
	return (_fd);
}

void Client::setFd(int fd)
{
	_fd = fd;
}

void Client::setIp(std::string ip)
{
	_ip = ip;
}

std::string Client::getNick()
{
	return (_nickname);
};

std::string Client::getUser()
{
	return (_user);
};

std::string Client::getPass()
{
	return (_pass);
};
