#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Server.hpp"

class Channel {
	private:
		std::string _name;
		std::vector<int> _sockcl;

	public:
		Channel(std::string _name);
		~Channel();
		void createChannel(Client *cl);
		void addClient(Client *cl);
		void clearClient(int cl_fd);
};

#endif