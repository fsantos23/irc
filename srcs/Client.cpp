#include "../includes/Client.hpp"

Client::Client(void) : _fd(-1), _ip(""), _nickname("*"), _user("*"), _pass("")
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