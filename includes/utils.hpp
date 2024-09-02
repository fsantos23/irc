#ifndef ULTILS_HPP
#define UTILS_HPP

#include <string>
#include <sys/socket.h>

void sendColoredMessage(int client_fd, const std::string& message, const std::string& colorCode);

#endif