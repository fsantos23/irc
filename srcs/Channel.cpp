/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: paulo <paulo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:50:36 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/17 20:15:57 by paulo            ###   ########.fr       */
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

// PFV
/*
bool Channel::isNewClient(int fd)
{
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
        if ((*it)->getFd() == fd)
            return false;
    }
    return true;
}
*/

bool Channel::isNewClient(int fd)
{
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (*it != NULL && (*it)->getFd() == fd)
			return (false);
	}
	return (true);
}


void Channel::addClient(Client* client)
{
	_clients.push_back(client);
	std::cout << "client joined: " << client->getNick() << std::endl;
}
// PFV
/*
void Channel::removeClientOperator(int cl_fd)
{
    // Remove client from the _clients map
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if ((*it)->getFd() == cl_fd) 
		{
            _clients.erase(it);  // Remove from vector
            break;  // Exit loop after removal
        }
    }
	// Remove the client from the list of operators
	std::vector<int>::iterator it_op = std::find(_operators.begin(), _operators.end(), cl_fd);
	if (it_op != _operators.end())
		_operators.erase(it_op);
}
*/

void Channel::removeClientOperator(int cl_fd)
{
	// Remove client from the _clients vector
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); /* sem incremento */)
	{
		if ((*it)->getFd() == cl_fd) 
		{
			delete *it;
			it = _clients.erase(it);
		}
		else 
		{
			++it;
		}
	}

	// Remove the client from the Operators list
	std::vector<int>::iterator it_op = std::find(_operators.begin(), _operators.end(), cl_fd);
	if (it_op != _operators.end())
		_operators.erase(it_op);
		
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
	{
		std::cout << "No invited clients." << std::endl;
	}
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
	{
		std::cout << "User Limit: " << _userLimit << std::endl;
	}
	else
	{
		std::cout << "User Limit: No limit" << std::endl;
	}
	
	std::cout << "Connected Clients (socket IDs): ";
	if (_clients.empty())
	{
		std::cout << "No clients connected." << std::endl;
	}
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

	std::cout << "Channel Operators (socket IDs): ";
	for (std::vector<int>::const_iterator it = _operators.begin(); it != _operators.end(); ++it)
	{
		std::cout << "@" << *it << " ";
	}
	std::cout << std::endl << "     ***************************" << std::endl;
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
	return (std::find(_operators.begin(), _operators.end(), cl->getFd()) != _operators.end());
}

bool Channel::addOperator(Client* cl)
{
	// Check if the client is not already an operator
	if (isOperator(cl))
	{
		// Server console MSG
		std::cout << "Client " << cl->getNick() << " is already an operator." << std::endl;
		return (false);
	}
	else
	{
		_operators.push_back(cl->getFd());
		// Server console MSG
		std::cout << cl->getNick() << " has been promoted to operator." << std::endl;
		return (true);
	}
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
	Client* firstClient = *_clients.begin();
	if (firstClient == NULL)
	{
		// Upgrades the first client in the list to operator privileges.
		std::cout << "No clients in the channel." << std::endl;
		return;
	}
	addOperator(firstClient);
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
	
	std::cout << "Send message to Channel: " << _name << std::endl;
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
		for(std::vector<int>::iterator it2 = _operators.begin(); it2 != _operators.end(); ++it2)
		{
			if(*it2 == (*it)->getFd())
				list += "@";
		}
		list += (*it)->getNick() + " ";
	}
	return (list);
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
	std::vector<int>::iterator it = std::find(_operators.begin(), _operators.end(), cl_fd);

	if (it != _operators.end())
		_operators.erase(it);
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