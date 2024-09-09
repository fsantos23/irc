#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <map>
#include <sstream>
#include <vector>

class Client {
	private:
		int _fd;
		std::string _ip;
		std::string _nickname;
		std::string _user;
		std::string _pass;

	public:
		Client();
		int getFd();
		void setFd(int fd);
		void setIp(std::string ip);
		std::string getIp() {return _ip;};
		std::string getNick() {return _nickname;};
		std::string getUser() {return _user;};
		std::string getPass() {return _pass;};
		void setNick(std::string str) {_nickname = str;};
		void setUser(std::string str) {_user = str;}
		void setPass(std::string str) {_pass = str;}
};

#endif