#include "../includes/Client.hpp"

Client::Client(void) : _fd(-1), _ip(0), _nickname(NULL), _user(NULL), _pass(NULL)
{
	std::cout << "Client created" << std::endl;
}

int Client::getFd( void )
{
	return _fd;
}

void Client::setFd( int fd )
{
	_fd = fd;
}

void Client::setIp( std::string ip )
{
	_ip = ip;
}