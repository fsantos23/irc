/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:50:36 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/12 12:11:45 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"

Channel::Channel(std::string _name) : _name(_name), _inviteOnly(false), _topicRestricted(false), _userLimit(0)
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
	_clients.push_back(cl);
}

bool Channel::isNewClient(int fd)
{
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
        if ((*it)->getFd() == fd)
            return false;
    }
    return true;
}

void Channel::addClient(Client* client)
{
	_clients.push_back(client);
	std::cout << "client joined: " << client->getNick() << std::endl;
}

void Channel::clearClient(int cl_fd)
{
    // Remove client from the _clients map
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if ((*it)->getFd() == cl_fd) 
		{
            _clients.erase(it);  // Remove from vector
            break;  // Exit loop after removal
        }
    }
    // Remove client from the _operators vector
    for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); ++it) {
        if ((*it)->getFd() == cl_fd)
		{
            _operators.erase(it);  // Remove from vector
            break;  // Exit loop after removal
        }
    }
}

void Channel::broadcast(Client* self, const std::string &msg)
{
	(void)self;
	std::vector<Client*>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if(self->getFd() != (*it)->getFd())
			sendMessageToClient((*it)->getFd(), msg);
	}
}

void Channel::listChannelInfo() const
{
	std::cout << std::endl << "     ***** Channel Details *****" << std::endl;
	std::cout << "Channel Name: " << _name << std::endl;

	std::cout << "Channel Topic Restricted: " << (isTopicRestricted() ? "+t (operators only)" : "-t (all)") << std::endl;
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

	if (_userLimit > 0)
		std::cout << "User Limit: " << _userLimit << std::endl;
	else
		std::cout << "User Limit: No limit" << std::endl;
	
	std::cout << "Connected Clients (socket IDs): ";
	if (_clients.empty())
		std::cout << "No clients connected." << std::endl;
	else
	{
		for (std::vector<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			if (isOperator(*it))
				std::cout << "@" << (*it)->getFd() << " ";
			else
				std::cout << (*it)->getFd() << " ";
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

bool Channel::isOperator(Client* cl) const
{
	for (std::vector<Client*>::const_iterator it = _operators.begin(); it != _operators.end(); ++it) {
        if ((*it)->getFd() == cl->getFd())
            return true;
    }
    return false;
}

void Channel::addOperator(Client* cl)
{
	_operators.push_back(cl);
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
	if (countOperators() == 0 && !_clients.empty())
	{
		// Upgrades the first client in the list to operator privileges.
		Client* firstClient = *_clients.begin();
		addOperator(firstClient);
		std::cout << firstClient->getNick() << " has been promoted to operator." << std::endl;
	}
}

void Channel::inviteClient(Client* cl)
{
	// Adds a client to the list of invited clients.
	_invitedClients[cl->getFd()] = cl;
	std::cout << "Client " << cl->getNick() << " has been invited to the channel." << std::endl;
}

bool Channel::isInvited(Client* cl)
{
	return (_invitedClients.find(cl->getFd()) != _invitedClients.end());
}

void Channel::removeClient(int cl_fd)
{
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if ((*it)->getFd() == cl_fd) {
            _clients.erase(it);
            break;
        }
    }
    
    for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); ++it) {
        if ((*it)->getFd() == cl_fd) {
            _operators.erase(it);
            break;
        }
    }
}

void Channel::setKey(const std::string& key)
{
	_key = key;
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
	std::vector<Client*>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		sendMessageToClient((*it)->getFd(), msg);
	}
}

std::string Channel::getClientList()
{
	std::string list = "";
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		for(std::vector<Client*>::iterator it2 = _operators.begin(); it2 != _operators.end(); ++it2)
		{
			if((*it2)->getFd() == (*it)->getFd())
				list += "@";
		}
		list += (*it)->getNick() + " ";
	}
	return list;
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

void Channel::setUserLimit(int limit)
{
	_userLimit = limit;
}

int Channel::getUserLimit() const
{
	return (_userLimit);
}

bool Channel::hasUserLimit() const
{
	return (_userLimit) > 0;
}

Client* Channel::getClientByName(const std::string& nick) const
{
	std::vector<Client*>::const_iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if ((*it)->getNick() == nick)
			return (*it);
	}
	return (NULL);
}

void Channel::removeOperator(int cl_fd)
{
	for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); ++it)
	{
        if ((*it)->getFd() == cl_fd)
		{
            _operators.erase(it);
            return;
        }
    }
}

Client* Channel::getClientByFd(int fd) const
{
	for (std::vector<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
        if ((*it)->getFd() == fd)
            return *it;
    }
    return NULL;
}