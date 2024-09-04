/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 11:36:01 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/03 11:36:03 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Client.hpp"

int main(int ac, char **av)
{
	if(ac != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
	}
	// fazer check do port passado para verificar se é valido ou não
	// PCC
	Server ser(std::atoi(av[1]), av[2]);
	try
	{
		ser.initServer();
	}
	catch (const std::exception &e)
	{
		std::cout << std::endl << RED << "Error: " << e.what() << WHI << std::endl;
		return 1;
	}
	return 0;
}