/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:50:36 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/25 11:49:59 by pviegas          ###   ########.fr       */
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
}

void Channel::removeClientOperator(int cl_fd)
{
	// Remove client from the _clients vector
	for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end();)
	{
		if ((*it)->getFd() == cl_fd) 
		{
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

// for debugging purposes
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
		for (std::vector<int>::const_iterator it = _invitedClients.begin(); it != _invitedClients.end(); ++it)
		{
			Client* client = getClientByFd(*it);
			
			if (client && isOperator(client))
				std::cout << "@" << *it << " ";
			else
				std::cout << *it << " ";
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
		std::cout << YEL << "Client " << cl->getNick() << " is already an operator." << WHI << std::endl;
		return (false);
	}
	else
	{
		_operators.push_back(cl->getFd());
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
		return;
	}
	addOperator(firstClient);
}

bool Channel::inviteClient(Client* cl)
{
	// Check if the client is not already an operator
	if (isInvited(cl))
	{
		// Server console MSG
		std::cout << YEL << "Client " << cl->getNick() << " is already Invited." << WHI << std::endl;
		return (false);
	}
	else
	{
		_invitedClients.push_back(cl->getFd());
		// Server console MSG
		std::cout << GRE << cl->getNick() << " has been Invited." << WHI << std::endl;
		return (true);
	}
}

bool Channel::isInvited(Client* cl) const
{
	return (std::find(_invitedClients.begin(), _invitedClients.end(), cl->getFd()) != _invitedClients.end());
}

void Channel::removeInvited(int cl_fd)
{
	std::vector<int>::iterator it = std::find(_invitedClients.begin(), _invitedClients.end(), cl_fd);

	if (it != _invitedClients.end())
		_invitedClients.erase(it);
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

void Channel::clearChannel()
{
	_clients.clear();
	_operators.clear();
	_invitedClients.clear();
	_inviteOnly = false;
	_key.clear();
	_topic.clear();
	_topicRestricted = false;
	_userLimit = 0;
}
