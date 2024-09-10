#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include "Server.hpp"
#include <sys/socket.h>

void sendColoredMessage(int client_fd, const std::string& message, const std::string& colorCode);
void	print_tokens(std::vector<std::string> str);
void	sendMessageToClient(int client_fd, const std::string& message);
std::vector<std::string>	split(std::string str, std::string delimiter);

#endif