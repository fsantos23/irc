#include "../includes/Server.hpp"

Server::Server(int port, const std::string password) : _port(port), _password(password), _sockfd(-1), _signal(false)
{

}

Server::~Server()
{
	closeFds(); //close all fds after server closes
}

void Server::initServer()
{
	serSocket();

	std::cout << GRE << "Server <" << _port << "> Connected" << WHI << std::endl;

	while(true)
	{
		signal(SIGINT, handleSignal());
	}
}

void Server::serSocket()
{
	_sockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket

	if (_sockfd == -1)
		throw(std::runtime_error("Socket creation error"));
	int opt = 1;
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) //set rule to reuse port
		throw(std::runtime_error("Socket reuse rule error"));
	
	struct sockaddr_in server_addr; //bind type
	server_addr.sin_family = AF_INET; //IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY; //bind to any IP address
	server_addr.sin_port = htons(_port); //bind to port

	if(bind(_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) //bind socket to port
		throw(std::runtime_error("Bind failed"));

	if(listen(_sockfd, SOMAXCONN) < 0) //listen for connection
		throw(std::runtime_error("Listen failed"));
}

void Server::closeFds()
{
	if(_sockfd != -1)
		close(_sockfd);
}

void Server::handleSignal(int signum)
{
	if(signum == SIGINT)
		throw(std::runtime_error("^C used"));
}