/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:50:20 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/12 12:23:38 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

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
		std::cout << std::endl << RED << "Error: " << e.what() << WHI << std::endl;
		ser.clearChannels();
		return 1;
	}
	return 0;
}