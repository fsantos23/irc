/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pveiga-c <pveiga-c@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:50:20 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/12 17:37:50 by pveiga-c         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

bool	invalid_arguments(int argc, char *argv[])
{
	if (argc != 3) {
		std::cout << RED "Usage: ./ircserv <port> <password>" WHI << std::endl;
		return (1);
	}

	std::string	port_num(argv[1]);
	if (port_num.find_first_not_of("0123456789") != std::string::npos)
	{
		std::cerr << RED "Port Number Has To Be Numbers Only" WHI << std::endl;
		return (1);
	}
	int	port = atoi(port_num.c_str());
	if (port < 1024 || port > 65535) // confirmar min max port
	{
		std::cerr << RED "Port Number Has To Be In Range [1024, 65535]" WHI << std::endl;
		return (1);
	}
	return (0);
}
//PCC
int main(int argc, char **argv)
{

	if (invalid_arguments(argc, argv))
		return (-1);
	try
	{
		Server server(atoi(argv[1]), std::string(argv[2]));
		server.initServer();
	}
	catch(std::exception &e) {
		std::cerr << e.what() << std::endl;
		return (-1);
	}

	return (0);
}