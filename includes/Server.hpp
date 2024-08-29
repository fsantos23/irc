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

class Server {
	private:
		int _sockfd;
		int _port;
		std::string _password;
		bool _signal;

	public:
		Server(int port, const std::string password);
		~Server();

		void initServer();
		void serSocket();
		void acceptNewClient();
		static void handleSignal(int signum);
		void closeFds();
		void clearClients(int fd);


};

#endif