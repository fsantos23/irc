#include "../includes/Client.hpp"

Client::Client(void) : _fd(-1), _ip(""), _nickname("*"), _user("*"), _pass("")
{
	std::cout << "Client created" << std::endl;
}

int Client::getFd( void ) const
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

bool Client::hasMessage() const
{
	return !_messages.empty();
}

std::string Client::getMessage()
{
	if (!_messages.empty()) {
		std::string message = _messages.front();
		_messages.pop();
		return message;
	}
	return ("");
}