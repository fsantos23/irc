#include "../includes/utils.hpp"

void sendColoredMessage(int client_fd, const std::string& message, const std::string& colorCode)
{
    std::string coloredMessage = colorCode + message + "\033[0m";
    send(client_fd, coloredMessage.c_str(), coloredMessage.length(), 0);
}