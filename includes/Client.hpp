#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"

class Client {
	private:
		int _fd;
		std::string _ip;

	public:
		Client();

		int getFd();
		void setFd(int fd);
		void setIp(std::string ip);
};

#endif