/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 13:09:57 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/03 17:17:58 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Command.hpp"
#include "../includes/Client.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Message.hpp"
#include "../includes/Channel.hpp"

void cmdJoin(std::vector<std::string> cmd, Client *cl)
{
	std::string message = "";
	if (cmd.size() == 1)
	{
		message = ERR_NEEDMOREPARAMS;
		message += " ";
		message += cl->getNick();
		message += " ";
		message += cmd[0];
		message += " :Not enough parameters given\r\n";
		sendMessageToClient(cl->getFd(), message);
		return ;
	}
	
	std::vector<std::string> channels = split(cmd[1], ",");
	std::vector<std::string>::iterator it;

	for (it = channels.begin(); it != channels.end(); it++)
	{
		if ((*it)[0] != '#')
		{
			message = ERR_NOSUCHCHANNEL;
			message += " ";
			message += cl->getNick();
			message += " ";
			message += *it;
			message += " :No such channel\r\n";
			sendMessageToClient(cl->getFd(), message);
			continue;
		}
		
		// PFV
		// procurar pelo canal, se não existir cria e coloca o cliente como operador
		// se existir adiciona o cliente
		// se o cliente já estiver no canal, não faz nada
	}
}