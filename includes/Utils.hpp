#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>

// -----Colors ----- //
#define RED "\e[1;31m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
#define BLU	"\e[1;34m"
#define PUR "\e[1;35m"
#define CYA "\e[1;36m"
#define WHI "\e[0;37m"
//-------------------//

void	sendColoredMessage(int client_fd, const std::string& message, const std::string& colorCode);
void	print_tokens(std::vector<std::string> str);
void	sendMessageToClient(int client_fd, const std::string& message);
std::vector<std::string>	split(std::string str, std::string delimiter);

#endif