#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Server.hpp"

class Channel {
	private:
		std::string _name;

	public:
		Channel(std::string _name);
		~Channel();
};

#endif