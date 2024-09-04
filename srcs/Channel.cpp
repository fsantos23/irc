/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 11:36:16 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/03 16:36:49 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"

Channel::Channel(std::string name) : _name(name)
{
}

Channel::~Channel()
{
}

void Channel::createChannel(Client *cl)
{
	this->_sockcl.push_back(cl->getFd());
}

void Channel::addClient(Client *cl)
{
	this->_sockcl.push_back(cl->getFd());
}

void Channel::clearClient(int cl_fd)
{
	std::vector<int>::iterator it = std::find(_sockcl.begin(), _sockcl.end(), cl_fd);
	if (it != _sockcl.end())
		_sockcl.erase(it);
}

void Channel::broadcast(Client* self, const std::string &msg)
{
	int self_fd = self->getFd();

	for (size_t i = 0; i < _sockcl.size(); ++i)
	{
		if (_sockcl[i] != self_fd)
		{
			sendMessageToClient(_sockcl[i], msg);
		}
	}
}