/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:52:32 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/12 10:52:33 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

//-----------------------------------------------//
#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
//-----------------------------------------------//

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
#include "Client.hpp"
#include "Utils.hpp"
#include "Channel.hpp"

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
		std::map<std::string, Channel*>	_channels;

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
		int checkEntry(std::vector<std::string> str, Client *cl);
		void mainCommands(std::vector<std::string> str, Client *cl);
		void manageChannels(std::vector<Channel> _ch);
		void sendError(int client_fd, const std::string& nickname, int error_code, const std::string& message);
		void sendMessageAll(std::string msg);
		void checkQuit(Client cl, std::vector<std::string> str);
		void sendtoChannel(Channel ch, std::string str);
		Channel* joinChannel(const std::string &name, Client *cl);
		bool isChannelExist(std::string channelName);
		Channel* getChannel(const std::string& name);
		

		//commands
		void PRIVMSG(std::vector<std::string> str, Client *cl);
		void JOIN(std::vector<std::string> cmd, Client *cl);
		void PART(std::vector<std::string> cmd, Client* cl);
		void INVITE(std::vector<std::string> cmd, Client* cl);
		void MODE(std::vector<std::string> cmd, Client* cl);
		void KICK(std::vector<std::string> cmd, Client* cl);
		void TOPIC(std::vector<std::string> cmd, Client* cl);
		// PFV
		void LISTINFO(std::vector<std::string> cmd, Client* cl);
};

#endif