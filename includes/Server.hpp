#ifndef SERVER_HPP
#define SERVER_HPP

//-----------------------------------------------//
#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
//-----------------------------------------------//

#include <sys/types.h>
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
#include <vector>
#include <poll.h>
#include <algorithm>
#include <string>
#include "Client.hpp"

class Client;

class Server {
	private:
		int _port;
		std::string _password;
		int _sockfd;
		bool _signal;
		int _sockcl;
		std::vector <Client> _cl;
		std::vector <struct pollfd> _pollfds;

	public:
		Server(int port, const std::string password);
		~Server();

		void initServer();
		void serSocket();
		void acceptNewClient();
		static void handleSignal(int signum);
		void closeFds();
		void clearClient(int fd);
		void handleClientMessage(int client_fd);
		void handleInput(std::vector<std::string> str, int client fd);
};

#endif