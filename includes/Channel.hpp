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
		int 						_limit;
		int							_users;

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
		int getUsers();
		void incrementUsers();
		void setLimit(int num);
		int getLimit();
		std::string getClientList();

		// Operators management
		bool isOperator(Client* cl) const;
		void addOperator(Client* cl);
		int countOperators();
		void forceOperator();

		// Invite management
		void inviteClient(Client* cl);
		bool isInvited(Client* cl);

		// Key management
		void setKey(const std::string& key);
		void removeKey();
		bool hasKey() const;
		bool checkKey(const std::string& key);

		// Utils
		void broadcast(Client* self, const std::string &msg);
		void sendMessageChannel(std::string msg);
		void listChannelInfo() const;
};

#endif