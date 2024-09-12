/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:50:36 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/12 10:50:38 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"

Channel::Channel(std::string name) : _name(name), _inviteOnly(false)
{
}

Channel::~Channel()
{
}

std::string	&Channel::getChannelName() 
{
	return (this->_name);
}

void Channel::createChannel(Client *cl)
{
	this->_clients.insert(std::make_pair(cl->getFd(), cl));
}

bool Channel::isNewClient(int fd)
{
	// Checks if the client is already in the channel.
	if (_clients.find(fd) != _clients.end())
		return (false);
	else
		return (true);
}

void Channel::addClient(int fd, Client* client)
{
	_clients[fd] = client;
}

void Channel::clearClient(int cl_fd)
{
	std::map<int, Client*>::iterator it = _clients.find(cl_fd);
	if (it != _clients.end())
		_clients.erase(it);
	std::map<int, Client*>::iterator it_op = _operators.find(cl_fd);
	if (it_op != _operators.end())
	_operators.erase(it_op);
}

std::map<int, Client*> Channel::getClient() const
{
	return (_clients);
}

Client* Channel::getClientByFd(int fd) const
{
	std::map<int, Client*>::const_iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		return (it->second);
	}
	return (NULL);
}

Client* Channel::getClientByName(const std::string& nick) const
{
	std::map<int, Client*>::const_iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		Client* client = it->second;
		if (client->getNick() == nick)
		{
			return (client);
		}
	}
	return (NULL);
}

void Channel::broadcast(Client* self, const std::string &msg)
{
	std::map<int, Client*>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if(self->getFd() != it->second->getFd())
			sendMessageToClient(it->second->getFd(), msg);
	}
}

void Channel::listChannelInfo() const
{
	std::cout << std::endl << "Channel Name: " << _name << std::endl;

	std::cout << "Channel Topic: " << (isTopicRestricted() ? "+t (operators only)" : "-t (all)") << std::endl;
	std::cout << "Channel Topic: " << _topic << std::endl;

	std::cout << "Channel Mode: " << (isInviteOnly() ? "+i (invite-only)" : "-i (open)") << std::endl;

	std::cout << "Channel Protected: " << (hasKey() ? "+k (Protected)" : "-k (Unprotected)") << std::endl;

	std::cout << "Invited Clients (socket IDs): ";
	if (_invitedClients.empty())
		std::cout << "No invited clients." << std::endl;
	else
	{
		for (std::map<int, Client*>::const_iterator it = _invitedClients.begin(); it != _invitedClients.end(); ++it)
		{
			if (isOperator(it->second))
				std::cout << "@" << it->first << " ";
			else
				std::cout << it->first << " ";
		}
		std::cout << std::endl;
	}

	std::cout << "Connected Clients (socket IDs): ";
	if (_clients.empty())
		std::cout << "No clients connected." << std::endl;
	else
	{
		for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			if (isOperator(it->second))
				std::cout << "@" << it->first << " ";
			else
				std::cout << it->first << " ";
		}
		std::cout << std::endl;
	}
}

void Channel::setInviteOnly(bool value)
{
	_inviteOnly = value;
}

bool Channel::isInviteOnly() const
{
	return (_inviteOnly);
}

bool Channel::isInvited(Client* cl) const
{
	return (_invitedClients.find(cl->getFd()) != _invitedClients.end());
}

bool Channel::isOperator(Client* cl) const
{
	return (_operators.find(cl->getFd()) != _operators.end());
}

void Channel::addOperator(int fd, Client* client)
{
	_operators[fd] = client;
}



void Channel::setMode(std::string mode, bool enable)
{
	// If (invite-only) mode is being set
	if (mode == "i")
	{
		setInviteOnly(enable);
		if (enable)
			std::cout << "Channel mode set to invite-only" << std::endl;
		else
			std::cout << "Invite-only mode disabled" << std::endl;
	}
	// PFV
	// Outros modos podem ser adicionados aqui no futuro
}

int Channel::countOperators()
{
	return (_operators.size());
}

int Channel::countClients()
{
	return (_clients.size());
}

void Channel::forceOperator()
{
	// Upgrades the first client in the list to operator privileges.
	Client* firstClient = _clients.begin()->second;
	addOperator(firstClient->getFd(), firstClient);
	std::cout << firstClient->getNick() << " has been promoted to operator." << std::endl;
}

void Channel::removeOperator(int cl_fd)
{
	std::map<int, Client*>::iterator it = _operators.find(cl_fd);

	if (it != _operators.end())
		_operators.erase(it);
}

void Channel::inviteClient(Client* cl)
{
	// Adds a client to the list of invited clients.
	_invitedClients[cl->getFd()] = cl;
	std::cout << "Client " << cl->getNick() << " has been invited to the channel." << std::endl;
}

void Channel::removeClient(int cl_fd)
{
	std::map<int, Client*>::iterator it = _clients.find(cl_fd);

	if (it != _clients.end())
		_clients.erase(it);
}

void Channel::setKey(const std::string& key)
{
	_key = key;
}

std::string Channel::getKey() const
{
	return (_key);
}

void Channel::removeKey()
{
	_key.clear();
}

bool Channel::hasKey() const
{
	return (!_key.empty());
}

bool Channel::checkKey(const std::string& key)
{
	std::cout << std::endl << "Checking key: " << key << std::endl;
	std::cout << "Stored key: " << _key << std::endl << std::endl;
	return (_key == key);
}

void Channel::sendMessageChannel(std::string msg)
{
	std::map<int, Client*>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
		sendMessageToClient(it->second->getFd(), msg);

}

void Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

std::string Channel::getTopic() const
{
	return (_topic);
}
void Channel::setTopicRestricted(bool value)
{
	_topicRestricted = value;
}
bool Channel::isTopicRestricted() const
{
	return (_topicRestricted);
}