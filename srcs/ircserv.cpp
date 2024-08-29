#include "Server.hpp"
#include "Client.hpp"

int main(int ac, char **av)
{
	if(ac != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
	}
	//fazer check do port passado para verificar se é valido ou não
	Server ser(std::atoi(av[1]), av[2]);
	try
	{
		ser.initServer();
	}
	catch (const std::exception &e)
	{
		std::cout << RED << "Error: " << e.what() << WHI << std::endl;
		return 1;
	}
	return 0;
}