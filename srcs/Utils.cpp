/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 11:34:53 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/03 17:18:43 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Utils.hpp"

void sendColoredMessage(int client_fd, const std::string& message, const std::string& colorCode)
{
	std::string coloredMessage = colorCode + message + "\033[0m";
	send(client_fd, coloredMessage.c_str(), coloredMessage.length(), 0);
}

void print_tokens(std::vector<std::string> str)
{
	std::vector<std::string>::iterator it = str.begin();
	std::vector<std::string>::iterator ite = str.end();
	int i = 1;

	std::cout << "Tokens: " << std::endl;
	while (it != ite)
	{
		std::cout << i << ") "<< (*it) << std::endl;
		++it;
		++i;
	}
}

void sendMessageToClient(int client_fd, const std::string& message)
{
	send(client_fd, message.c_str(), message.length(), 0);
}

/**
 * Splits a string into multiple substrings based on a delimiter.
 * 
 * @param str The string to be split.
 * @param delimiter The delimiter used to split the string.
 * @return A vector of strings containing the substrings.
 */
std::vector<std::string>	split(std::string str, std::string delimiter)
{
	std::cout << GRE << str << WHI << std::endl;
	std::vector<std::string> tokens;
	size_t pos = 0;

	// Split the string into substrings based on the delimiter and add to the vector.
	while ((pos = str.find(delimiter)) != std::string::npos)
	{
		if (str[0] == ':')
		{
			tokens.push_back(str.substr(1, std::string::npos));
			str.clear();
			break;
		}
		tokens.push_back(str.substr(0, pos));
		str.erase(0, pos + delimiter.length());
	}
	
	// If the string is not empty, add the remaining part to the vector.
	if (!str.empty())
	{
		if (str[0] == ':')
		{
			tokens.push_back(str.substr(1, std::string::npos));
			str.clear();
		}
		else
		{
			tokens.push_back(str);
			str.clear();
		}
	}
	
	// PFV
	print_tokens(tokens);
	
	return (tokens);
}