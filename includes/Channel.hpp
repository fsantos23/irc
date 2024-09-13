/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: paulo <paulo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:52:06 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/13 12:11:08 by paulo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <algorithm>
#include "Server.hpp"

class Channel
{
	private:
		std::string					_name;
		std::map<int, Client *>		_clients;
		std::vector<int>			_operators;
		bool						_inviteOnly;
		std::map<int, Client *>		_invitedClients;
		std::string					_key;
		std::string					_topic;
		bool						_topicRestricted;
		int							_userLimit;

	public:
		Channel(std::string _name);
		~Channel();

		// Channel management
		std::string& getChannelName();
		void setInviteOnly(bool value);
		bool isInviteOnly() const;
		void setTopic(const std::string& topic);
		std::string getTopic() const;
		void setTopicRestricted(bool value);
		bool isTopicRestricted() const;

		// Client management
		bool isNewClient(int fd);
		void addClient(int fd, Client* client);
		void clearClient(int cl_fd);
		int countClients();
		void removeClient(int cl_fd);
		std::string getClientList();
		Client* getClientByName(const std::string& nick) const;
		Client* getClientByFd(int fd) const;

		// Operators management
		bool isOperator(Client* cl) const;
		void addOperator(Client* cl);
		int countOperators();
		void forceOperator();
		void removeOperator(int cl_fd);

		// Invite management
		void inviteClient(Client* cl);
		bool isInvited(Client* cl);

		// Key management
		void setKey(const std::string& key);
		void removeKey();
		bool hasKey() const;
		bool checkKey(const std::string& key);

		// User limit management
		void setUserLimit(int limit);
		int getUserLimit() const;
		bool hasUserLimit() const;

		// Utils
		void broadcast(Client* self, const std::string &msg);
		void sendMessageChannel(std::string msg);
		void listChannelInfo() const;
};

#endif