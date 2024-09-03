/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 17:19:58 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/03 17:20:01 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <poll.h>
#include <algorithm>
#include <string>

#include "../includes/Client.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Command.hpp"

class Client;
class Channel;

class Server
{
	private:
		int _port;
		std::string _password;
		int _sockfd;
		bool _signal;
		int _sockcl;
		std::vector <Client> _cl;
		std::vector <struct pollfd> _pollfds;
		std::vector<Channel> _ch;

	public:
		Server(int port, const std::string password);
		~Server();

		void initServer();
		void serSocket();
		void acceptNewClient();
		static void handleSignal(int signum);
		void closeFds();
		void clearClient(int fd);
		void handleClientMessage(int client_fd);
		void handleInput(std::vector<std::string> str, int client_fd);
		int checkIfRegistered(std::vector<std::string> str, Client *cl);
		void mainCommands(std::vector<std::string> str, Client *cl);
		void manageChannels(std::vector<Channel> _ch);
};

#endif