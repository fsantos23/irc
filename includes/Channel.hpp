#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Server.hpp"

class Channel
{
	private:
		std::string					_name;
		std::map<int, Client *>		_clients;
		std::map<int, Client *>		_operators;
		bool						_inviteOnly;
		std::map<int, Client *>		_invitedClients;
		std::string					_key;

	public:
		Channel(std::string _name);
		~Channel();

		// Channel management
		std::string& getChannelName();
		void createChannel(Client *cl);
		void setInviteOnly(bool value);
		bool isInviteOnly() const;
		void setMode(std::string mode, bool enable);

		// Client management
		bool isNewClient(int fd);
		void addClient(int fd, Client* client);
		void clearClient(int cl_fd);
		int countClients();
		void removeClient(int cl_fd);
		std::map<int, Client*> getClient() const;
		Client* getClientByFd(int fd) const;
		Client* getClientByName(const std::string& nick) const;

		// Operators management
		bool isOperator(Client* cl) const;
		void addOperator(int fd, Client* client);
		int countOperators();
		void forceOperator();
		void removeOperator(int fd);

		// Invite management
		void inviteClient(Client* cl);
		bool isInvited(Client* cl) const;

		// Key management
		void setKey(const std::string& key);
		std::string getKey() const;
		void removeKey();
		bool hasKey() const;
		bool checkKey(const std::string& key);

		// Utils
		void broadcast(Client* self, const std::string &msg);
		void sendMessageChannel(std::string msg);
		void listChannelInfo() const;
};

#endif