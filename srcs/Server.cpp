/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: paulo <paulo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:50:46 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/17 20:25:34 by paulo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server(int port, const std::string password) : _port(port), _password(password), _sockfd(-1), _sockcl(0)
{

}

Server::~Server()
{
	/* closeFds(); //close all fds after server closes
	closeChannels(); //close all channels after server closes
	closeClients(); //close all clients after server closes */
}

bool Server::_signal = true;

void Server::initServer()
{
	serSocket();

	std::cout << GRE << "Server Running and listening on port " << _port << std::endl << "Waiting for connections..." << WHI << std::endl;
	_signal = true;
	while (_signal)
	{
		signal(SIGINT, handleSignal);
		signal(SIGQUIT, handleSignal);
		
		int poll_count = poll(_pollfds.data(), _pollfds.size(), -1);

		if (poll_count < 0)
		{
			std::cerr << "poll() error." << std::endl;
			break;
		}
		if (poll_count == 0)
			continue;
		for (std::vector<pollfd>::size_type i = 0; i < _pollfds.size(); ++i) 
		{
			pollfd& pfd = _pollfds[i];

			if (pfd.revents & POLLIN)
			{
				if (pfd.fd == _sockfd)
					acceptNewClient();
				else
					handleClientMessage(pfd.fd);
			}
		}
	}
	/* closeClients(); */
	clearChannels();
	closeFds();
}

void Server::serSocket()
{
	_sockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket

	if (_sockfd == -1)
		throw(std::runtime_error("Socket creation error"));
	int opt = 1;
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) //set rule to reuse port
		throw(std::runtime_error("Socket reuse rule error"));
	
	struct sockaddr_in server_addr; //bind type
	server_addr.sin_family = AF_INET; //IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY; //bind to any IP address
	server_addr.sin_port = htons(_port); //bind to port

	if(bind(_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) //bind socket to port
		throw(std::runtime_error("Bind failed"));

	if(listen(_sockfd, SOMAXCONN) < 0) //listen for connection
		throw(std::runtime_error("Listen failed"));


	struct pollfd server_pollfd;
    server_pollfd.fd = _sockfd;
    server_pollfd.events = POLLIN;
    server_pollfd.revents = 0;
	_pollfds.push_back(server_pollfd);
}

void Server::closeFds() {
    for (std::vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it) {
        if (it->fd >= 0) {
            close(it->fd);
        }
    }
}

/*
void Server::closeFds()
{
	if(_sockfd != -1)
		close(_sockfd);
	for(int i = 0; i < _sockcl; i++)
	{
		if(_cl[i].getFd() != -1)
			close(_cl[i].getFd());
	}
}
*/

void Server::handleSignal(int signum)
{
	(void)signum;
	throw(std::runtime_error("Server shuting down"));
}

/* void Server::closeChannels()
{
	
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second; // Libere a memória alocada para o canal
	_channels.clear(); // Limpe o mapa de canais
} */

/* void Server::closeClients()
{
    for (std::vector<Client>::iterator it = _cl.begin(); it != _cl.end(); ++it)
    {
        close((it)->getFd());
    }
    _cl.clear();
} */

void Server::acceptNewClient()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_sock = accept(_sockfd, (struct sockaddr *)&client_addr, &client_len); //accept new client
	if(client_sock < 0)
		throw(std::runtime_error("Connecting client fail"));

	Client newClient;
	// PFV
//	newClient.setIp(inet_ntoa(client_addr.sin_addr));
	newClient.setIp(std::string(inet_ntoa(client_addr.sin_addr)));

	newClient.setFd(client_sock);
	_cl.push_back(newClient); // add new client to vector

	std::cout << GRE << "Client connected: " << inet_ntoa(client_addr.sin_addr) << WHI << std::endl;

	struct pollfd client_pollfd;
	client_pollfd.fd = client_sock;
	client_pollfd.events = POLLIN;
	client_pollfd.revents = 0;
	_pollfds.push_back(client_pollfd);

	_sockcl++;
}

// PFV
/*
void Server::clearClient(int fd)
{
	// Check if the client is in any channel
	for (std::map<std::string, Channel*>::iterator chan_it = _channels.begin(); chan_it != _channels.end(); ++chan_it)
	{
		Channel* channel = chan_it->second;

		// Check if the client is in the channel
		Client* client = channel->getClientByFd(fd);
		if (client)
		{
			channel->removeClientOperator(fd);
			std::cout << "Client " << fd << " removed from channel " << channel->getChannelName() << std::endl;
		}
		if (channel->countOperators() == 0 && channel->countClients() > 0)
		{
			channel->forceOperator();
		}
	}
	for (std::vector<Client>::iterator it = _cl.begin(); it != _cl.end();)
	{
		// Check if the client is in the Server client list
		if (it->getFd() == fd)
		{
			// Close the client socket
			close(fd);
			// Remove from the client list
			//_cl.erase(it);
			it = _cl.erase(it);

			// Remove the pollfd entry for the client
			for (std::vector<pollfd>::iterator poll_it = _pollfds.begin(); poll_it != _pollfds.end(); ++poll_it) 
			{
                if (poll_it->fd == fd)
				{
					_pollfds.erase(poll_it);
					break;
				}
			}
			break;
		}
		else
		{
			++it;  // Continua iterando
		}
	}
}
*/

void Server::clearClient(int fd, std::string msg)
{
	// Check if the client is in any channel
	for (std::map<std::string, Channel*>::iterator chan_it = _channels.begin(); chan_it != _channels.end(); ++chan_it)
	{
		Channel* channel = chan_it->second;
		Client* client = channel->getClientByFd(fd);
		if (client)
		{
			channel->removeClientOperator(fd);
			// MSG to Server Console
			std::cout << "Client " << fd << " removed from channel " << channel->getChannelName() << std::endl;

			// Send a message to the channel informing that the client has left
			channel->sendMessageChannel(msg);

			// Check if the channel has no operators and still has clients
			if (channel->countOperators() == 0 && channel->countClients() > 0)
			{
				channel->forceOperator();
			}
		}
	}

	// Remove the client from the Server clients list
	for (std::vector<Client>::iterator it = _cl.begin(); it != _cl.end();)
	{
		if (it->getFd() == fd)
		{
			close(fd);
			// Remove the client from the clients list
			it = _cl.erase(it);

			// Remove the pollfd entry for the client
			for (std::vector<pollfd>::iterator poll_it = _pollfds.begin(); poll_it != _pollfds.end(); /* sem incremento */)
			{
				if (poll_it->fd == fd)
				{
					poll_it = _pollfds.erase(poll_it);  // Ajusta o iterador após a remoção
				}
				else
				{
					++poll_it;
				}
			}
			break;
		}
		else
		{
			++it;
		}
	}
}

void Server::handleClientMessage(int client_fd)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_received < 0)
		throw(std::runtime_error("Failed receiving data from client."));
	else if (bytes_received == 0)
	{
		std::cout << "Client disconnected." << std::endl;
		clearClient(client_fd, "Client disconnected");
		return;
	}
	buffer[bytes_received] = '\0';

	std::istringstream stream(buffer);
	std::string line;

	// Read each line separated by "\r\n"
	while (std::getline(stream, line)) 
	{
		// Remove any trailing carriage return manually
		if (line.size() > 0 && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);  // Remove the last character
		if (!line.empty())
		{
			// Split the line into parts based on spaces
			std::vector<std::string> commandParts;
			std::istringstream lineStream(line);
			std::string part;

			while (lineStream >> part)
			{
				commandParts.push_back(part);
			}
			// Process the command
			handleInput(commandParts, client_fd);
		}
	}

}

void Server::handleInput(std::vector<std::string> str, int client_fd)
{
	int j = 0;

	for(size_t i = 0; i < _cl.size(); i++)
	{
		if(_cl[i].getFd() == client_fd)
		{
			j = i;
			break;
		}
	}
	if(QUIT(_cl[j], str))
		return ;
	if(checkEntry(str, &_cl[j]))
		return ;
	mainCommands(str, &_cl[j]);
}

int Server::checkEntry(std::vector<std::string> str, Client *cl)
{
	std::string entry_array[] = {"USER", "NICK", "PASS"};
	std::vector<std::string> entry(entry_array, entry_array + 3);

	std::string user_array[] = {cl->getUser(), cl->getNick(), cl->getPass()};
	std::vector<std::string> user(user_array, user_array + 3);

	for (int i = 0; i < 3; i++)
	{
		if (str[0] == entry[i])
		{
			if(str.size() < 2)
			{
				std::string errorMessage = ":42_IRC 461 " + cl->getNick() + " " + entry[i] + " :Not enough parameters\r\n";
				send(cl->getFd(), errorMessage.c_str(), errorMessage.length(), 0);
			}
			else if (i == 1)
				checkNick(str[1], cl);
			else if (user[i] == "*" && i == 0)
			{
				checkUser(str, cl);
				std::cout << cl->getFd() << " changed their User to " << cl->getUser() << std::endl;
			}
			else if (i == 2 && str[1] == _password && user[i].empty())
			{
				cl->setPass(str[1]);
				std::cout << cl->getFd() << " changed their Pass to " << cl->getPass() << std::endl;
			}
			else if (i == 2 && str[1] != _password && user[i].empty())
				return (1);
			else
			{
				sendMessageClient(cl->getFd(), cl->getNick(), 462, ":You may not reregister");
				return (1);
			}
			return (1);
		}
	}
	for (int i = 0; i < 3; i++)
    {
        if (user[i] == "*" || user[i].empty())
        {
            sendMessageClient(cl->getFd(), cl->getNick(), 451, ":You have not registered");
            return 1;
        }
    }
    return 0;
}

void Server::checkNick(std::string str, Client *cl)
{

	std::string lowerNick = toLowerCase(str);
	for (std::vector<Client>::const_iterator it = _cl.begin(); it != _cl.end(); ++it)
	{
		if (toLowerCase(it->getNick()) == lowerNick)
			return ;
	}
	cl->setNick(str);
	std::cout << cl->getFd() << " changed their Nick to " << cl->getNick() << std::endl;
}

void Server::checkUser(std::vector<std::string> str, Client* cl)
{
	if(str.size() < 5)
	{
		std::string errorMessage = ":42_IRC 461 " + cl->getNick() + " USER :Not enough parameters\r\n";
		send(cl->getFd(), errorMessage.c_str(), errorMessage.length(), 0);
		return ;
	}
	if (str[4][0] != ':')
	{
		std::string errorMessage = ":42_IRC 461 " + cl->getNick() + " USER :Invalid realname format\r\n";
		send(cl->getFd(), errorMessage.c_str(), errorMessage.length(), 0);
		return ;
	}
	// delete the ':' from the string
	str[4].erase(0, 1);
	cl->setUser(str[4]);
}

void Server::mainCommands(std::vector<std::string> str, Client *cl)
{
	typedef void (Server::*CommandHolder)(std::vector<std::string>, Client*);
	std::map<std::string, CommandHolder> commandhandler;

	commandhandler["JOIN"] = &Server::JOIN;
	commandhandler["PRIVMSG"] = &Server::PRIVMSG;
	commandhandler["PART"] = &Server::PART;
	commandhandler["INVITE"] = &Server::INVITE;
	commandhandler["MODE"] = &Server::MODE;
	commandhandler["KICK"] = &Server::KICK;
	commandhandler["TOPIC"] = &Server::TOPIC;
	// for debugging
	commandhandler["LCI"] = &Server::LCI;

	std::map<std::string, CommandHolder>::iterator it = commandhandler.find(str[0]);
	if(it != commandhandler.end())
		(this->*(it->second))(str, cl);
	else
		sendMessageClient(cl->getFd(), cl->getNick(), 421, ":Unknown command");
}

void Server::sendMessageClient(int client_fd, const std::string& nickname, int error_code, const std::string& message)
{
	std::string server_name = "42_IRC";
	std::ostringstream oss;
	oss << ":" << server_name << " " << error_code << " " << nickname << " " << message << "\r\n";
	std::string error_message = oss.str();
	send(client_fd, error_message.c_str(), error_message.length(), 0);
}

void Server::sendMessageAll(std::string msg)
{
	msg += "\r\n";
	for(size_t i = 0; i < _cl.size(); i++)
		send(_cl[i].getFd(), msg.c_str(), msg.size(), 0);
}

bool Server::QUIT(Client cl, std::vector<std::string> str)
{
	if (str[0] == "QUIT") 
	{
		// Msg to Server Console 
		std::cout << cl.getNick() << " has quit" << std::endl;

		std::string quitMessage = ":" + cl.getNick() + "!" + cl.getUser() + "@" + cl.getIp() + " QUIT ";
		if (str.size() > 1)
		{
			for (size_t i = 1; i < str.size(); ++i)
				quitMessage += str[i] + " ";

	// PFV			
	//		if (quitMessage[0] == ':')
	//			quitMessage = quitMessage.substr(1);
				
	//		quitMessage = quitMessage.substr(0, quitMessage.size() - 1);
			quitMessage += "\r\n";
		} 
		else
		{
			quitMessage += ":Client Quit\r\n";
//			sendMessageAll(cl.getNick() + " has quit (Client disconnected)");
		}
//		sendMessageAll(quitMessage);
		clearClient(cl.getFd(), quitMessage);
	
		return 1;
	}
	return 0;
}

void Server::PRIVMSG(std::vector<std::string> str, Client *cl)
{
    std::string sender = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " PRIVMSG ";
    if (str.size() < 3) 
    {
        sendMessageClient(cl->getFd(), cl->getNick(), 411, ":No recipient given");
        return;
    }
    std::string fullMessage;
    for (size_t i = 2; i < str.size(); ++i)
    {
        fullMessage += str[i];
        if (i < str.size() - 1)
            fullMessage += " ";
    }
	bool recipientFound = false;
    if (str[1][0] == '#') // Message to a channel
    {
		for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			// Check if the channel name matches the target channel in str[1]
			if (it->first == str[1])
			{
				std::string message = sender + str[1] + " " + fullMessage + "\r\n";
				it->second->broadcast(cl, message);
				recipientFound = true;
			}
		}
    }
	else
	{
		for (size_t i = 0; i < _cl.size(); ++i)
		{
			if (str[1] == _cl[i].getNick())
			{
				std::string message = sender + str[1] + " " + fullMessage + "\r\n";
				send(_cl[i].getFd(), message.c_str(), message.length(), 0);
				recipientFound = true;
				break;
			}
		}
	}
	if (!recipientFound)
		sendMessageClient(cl->getFd(), cl->getNick(), 401, ":No such nick/channel");
}

void Server::JOIN(std::vector<std::string> cmd, Client *cl)
{
	// Check if there are enough parameters for the JOIN command.
	if (cmd.size() < 2)
	{
		std::string errorMessage = ":42_IRC 461 " + cl->getNick() + " JOIN :Not enough parameters\r\n";
		send(cl->getFd(), errorMessage.c_str(), errorMessage.length(), 0);
		return;
	}

	// Split the channel names.
	std::vector<std::string> channels = split(cmd[1], ",");
	std::string key = (cmd.size() > 2) ? cmd[2] : "";
	std::vector<std::string> keypass = split(key, ",");

	// Iterator for keys
	std::vector<std::string>::iterator key_it = keypass.begin();

	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		// Ensure the channel name starts with '#'.
		if ((*it)[0] != '#')
		{
			sendMessageClient(cl->getFd(), cl->getNick(), 403, *it + " :No such channel");
			continue;
		}

		// Join or create the channel.
		Channel* channel = joinChannel(*it, cl);
		
		// Check if the channel is invite-only and if the client is allowed to join.
		if (channel->isInviteOnly() && !channel->isInvited(cl))
		{
			sendMessageClient(cl->getFd(), cl->getNick(), 473, *it + ":Cannot join channel (+i)");
			continue;
		}

		// Validate the channel key if necessary.
		if (channel->hasKey() && !channel->checkKey(*key_it))
		{
			sendMessageClient(cl->getFd(), cl->getNick(), 475, *it + ":Cannot join channel (+k)");
			continue;
		}

		// Check if the client is already in the channel.
		if (!channel->isNewClient(cl->getFd()))
			continue;

		// Check if the channel has a user limit and if it has been reached.
		if (channel->hasUserLimit() && channel->countClients() >= channel->getUserLimit())
		{
			sendMessageClient(cl->getFd(), cl->getNick(), 471, *it + ":Cannot join channel (+l)");
			continue;
		}
		
		// Add the client to the channel.
		channel->addClient(cl);
		std::cout << "Client " << cl->getFd() << " joined channel " << *it << std::endl;

		// 1. Broadcast the JOIN message to the channel.
		std::string joinMessage = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + *it + "\r\n";
		channel->sendMessageChannel(joinMessage);


		std::string nameList = channel->getClientList();
		std::string nameReply = ":42_IRC 353 " + cl->getNick() + " = " + *it + " :" + nameList + "\r\n";
		sendMessageToClient(cl->getFd(), nameReply);

		std::string endOfNamesReply = ":42_IRC 366 " + cl->getNick() + " " + *it + " :End of /NAMES list\r\n";
		sendMessageToClient(cl->getFd(), endOfNamesReply);

		if (key_it != keypass.end())
			++key_it;

		channel->listChannelInfo();
	}
}

Channel* Server::joinChannel(const std::string& name, Client *cl)
{
	if (isChannelExist(name))
		return _channels[name];

	// If the channel does not exist, creates a new one.
	Channel* newChannel = new Channel(name);
	_channels[name] = newChannel;
	std::cout << "Client " << cl->getFd() << " created channel " << name << std::endl;

	// Add the client as the operator of the newly created channel
	newChannel->addOperator(cl);
	std::cout << "Client " << cl->getFd() << " is the operator of channel " << name << std::endl;

	// Send a message to the client informing them that they are the operator
	sendMessageClient(cl->getFd(), cl->getNick(), 324, name + " +o");
	sendMessageClient(cl->getFd(), cl->getNick(), 331, name + " :No topic is set");
	
	return (newChannel);
}

void Server::PART(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 2)
	{
		// Not enough parameters
		sendMessageClient(cl->getFd(), cl->getNick(), 461, "PART :Not enough parameters");
		return;
	}

	std::vector<std::string> channels = split(cmd[1], ",");
	std::vector<std::string>::iterator it;

	for (it = channels.begin(); it != channels.end(); ++it)
	{
		if ((*it)[0] != '#')
		{
			// Invalid channel name
			sendMessageClient(cl->getFd(), cl->getNick(), 403, *it + " :No such channel");
			continue;
		}

		Channel* channel = getChannel(*it);
		if (!channel)
		{
			// Channel does not exist
			sendMessageClient(cl->getFd(), cl->getNick(), 403, *it + " :No such channel");
			continue;
		}

		if (channel->isNewClient(cl->getFd()))
		{
			// Client is not on the channel
			std::cout << "Client : " << cl->getFd() << " is not on the channel" << std::endl;
			sendMessageClient(cl->getFd(), cl->getNick(), 442, *it + " :You're not on that channel");
			continue;
		}

		// Broadcast a message to the channel notifying other users that the client has left
		std::string message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + *it + "\r\n";
		channel->sendMessageChannel(message);

		// Remove the client from the channel
		channel->removeClientOperator(cl->getFd());
		std::cout << "Client : " << cl->getFd() << " left the channel " << channel->getChannelName() << std::endl;

		if (channel->countOperators() == 0 && channel->countClients() > 0)
		{
			channel->forceOperator();
		}

		// for debugging
		channel->listChannelInfo();
	}
}

void Server::MODE(std::vector<std::string> cmd, Client* cl)
{
	if(cmd.size() == 2)
	{
		for(std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			if(it->first == cmd[1])
			{
				std::string msg = ":42_IRC MODE " + cmd[1] + " ";
				if(it->second->getUserLimit())
					msg += "+l ";
				if(!it->second->getKey().empty())
					msg += "+k ";
				if(!it->second->getTopic().empty())
					msg += "+t";
				msg += "\r\n";
				send(cl->getFd(), msg.c_str(), msg.length(), 0);
			return;
			}
		}
	}

	if (cmd.size() < 3)
	{
		sendMessageClient(cl->getFd(), cl->getNick(), 461, cmd[0] + " :Not enough parameters given");
		return;
	}

	// Extract channel name and mode
	std::string channelName = cmd[1];
	std::string mode = cmd[2];

	// Extract key, nick or user limit
	std::string key_nick_or_limit = (cmd.size() > 3) ? cmd[3] : "";

	// Check if the channel name is valid (must start with '#')
	if (channelName[0] != '#')
	{
		sendMessageClient(cl->getFd(), cl->getNick(), 403, channelName + " :No such channel");
		return;
	}

	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendMessageClient(cl->getFd(), cl->getNick(), 403, channelName + " :No such channel");
		return;
	}

	// Check if the client has operator permissions in this channel.
	if (!channel->isOperator(cl))
	{
		sendMessageClient(cl->getFd(), cl->getNick(), 482, channelName + " :You're not channel operator");
		return;
	}

	std::string message;
	// Handle invite-only mode (+i/-i)
	if (mode == "+i")
	{
		channel->setInviteOnly(true);
		message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + mode + "\r\n";
		channel->sendMessageChannel(message);
	}
	else if (mode == "-i")
	{
		channel->setInviteOnly(false);
		message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + mode + "\r\n";
		channel->sendMessageChannel(message);
	}
	// Handle key mode (+k/-k)
	else if (mode == "+k")
	{
		if (key_nick_or_limit.empty())
		{
			sendMessageClient(cl->getFd(), cl->getNick(), 461, cmd[0] + mode + " :Not enough parameters\r\n");
			return;
		}
		channel->setKey(key_nick_or_limit);
		message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + " " + mode + " " + key_nick_or_limit + "\r\n";
		channel->sendMessageChannel(message);
	}
	else if (mode == "-k")
	{
		channel->removeKey();
		message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + " " + mode + " " + key_nick_or_limit + "\r\n";
		channel->sendMessageChannel(message);
	}
	// Handle user limit mode (+l/-l)
	else if (mode == "+l")
	{
		if (key_nick_or_limit.empty())
		{
			sendMessageClient(cl->getFd(), cl->getNick(), 461, channelName + " " + mode + " :Not enough parameters");
			return;
		}
		if (!isdigit(key_nick_or_limit[0]))
		{
			sendMessageClient(cl->getFd(), cl->getNick(), 461, channelName + " " + mode + " :Not a valid limit");
			return;
		}		
		int userLimit = atoi(key_nick_or_limit.c_str());
		channel->setUserLimit(userLimit);
		message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + " " + mode + " " + key_nick_or_limit + "\r\n";
		channel->sendMessageChannel(message);
	}
	else if (mode == "-l")
	{
		channel->setUserLimit(0);
		message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + " " + mode + " " + key_nick_or_limit + "\r\n";
		channel->sendMessageChannel(message);
	}
	// Handle operator mode (+o/-o)
	else if (mode == "+o")
	{
		Client* client = channel->getClientByName(key_nick_or_limit);
		if (client)
		{
			if(channel->addOperator(client))
			{
				message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + " " + mode + " " + key_nick_or_limit + "\r\n";
				channel->sendMessageChannel(message);
			}
		}
		else
		{
			sendMessageClient(cl->getFd(), cl->getNick(), 401, key_nick_or_limit + " :No such nick\r\n");
		}
	}
	else if (mode == "-o")
	{
		Client* client = channel->getClientByName(key_nick_or_limit);
		if (client)
		{
			channel->removeOperator(client->getFd());
			message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + " " + mode + " " + key_nick_or_limit + "\r\n";
			channel->sendMessageChannel(message);
		}
		else
			sendMessageClient(cl->getFd(), cl->getNick(), 401, key_nick_or_limit + " :No such nick\r\n");
    }
    // Handle topic restriction mode (+t/-t)
    else if (mode == "+t")
    {
        channel->setTopicRestricted(true);
        sendMessageToClient(cl->getFd(), ":42_IRC 324 " + cl->getNick() + " " + channelName + " :Topic change restricted to operators\r\n");
    }
    else if (mode == "-t")
    {
        channel->setTopicRestricted(false);
        sendMessageToClient(cl->getFd(), ":42_IRC 324 " + cl->getNick() + " " + channelName + " :Anyone can change the topic\r\n");
    }
    else
        sendMessageClient(cl->getFd(), cl->getNick(), 472, mode + " :Unknown mode");

    // For debugging purposes
    channel->listChannelInfo();
}


void Server::INVITE(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 3)
	{
		// Not enough parameters
		sendMessageToClient(cl->getFd(), ":42_IRC 461 " + cl->getNick() + " " + cmd[0] + " :Not enough parameters given\r\n");
		return;
	}

	// Extract nickname and channel name
	std::string invitedNick = cmd[1];
	std::string channelName = cmd[2];

	// Checks if the channel name is valid (must start with '#')
	if (channelName[0] != '#')
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Checks if the channel exists
	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Checks if the client is an operator in the channel
	if (!channel->isOperator(cl))
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 482 " + cl->getNick() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	// Checks if the client to be invited exists
	Client* invitedClient = NULL;
	for (std::vector<Client>::iterator it = _cl.begin(); it != _cl.end(); ++it)
	{
		if (it->getNick() == invitedNick)
		{
			invitedClient = &(*it);
			break;
		}
	}
	
	if (!invitedClient)
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 401 " + cl->getNick() + " " + invitedNick + " :No such nick\r\n");
		return;
	}

	// Checks if the client is already in the channel
	if (!channel->isNewClient(invitedClient->getFd()))
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 443 " + cl->getNick() + " " + invitedNick + " " + channelName + " :is already on channel\r\n");
		return;
	}

	// Add the client to the channel invited clients list
	channel->inviteClient(invitedClient);

	std::string inviteMessage = ":42_IRC 341 " + cl->getNick() + " " + invitedClient->getNick() + " " + channelName + "\r\n";
	sendMessageToClient(cl->getFd(), inviteMessage);
	std::string confirmationMessage = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + invitedClient->getNick() + " " + channelName + "\r\n";
	sendMessageToClient(invitedClient->getFd(), confirmationMessage);

	channel->listChannelInfo();
}

void Server::KICK(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 3)
	{
		// Not enough parameters
		sendMessageToClient(cl->getFd(), ":42_IRC 461 " + cl->getNick() + " " + cmd[0] + " :Not enough parameters given\r\n");
		return;
	}

	std::string channelName = cmd[1];
	std::string targetNick = cmd[2];
	std::string reason = (cmd.size() > 3) ? cmd[3] : "";

	// Checks if the channel exists
	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Check if Client is in the channel
	Client* targetClient = NULL;
	for (size_t i = 0; i < _cl.size(); ++i)
	{
		if (_cl[i].getNick() == targetNick)
		{
			targetClient = &_cl[i];
			break;
		}
	}

	if (!targetClient)
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 403 " + cl->getNick() + " " + channelName + " :No such nick in channel\r\n");
		return;
	}

	// Checks if the client is an operator in the channel
	if (!channel->isOperator(cl))
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 403 " + cl->getNick() + " " + cmd[0] + " :You are not an operator in this channel.\r\n");
		return;
	}

	// Sends a message to all clients in the channel, informing about the KICK
	std::string kickMessage = ":" + cl->getNick() + " KICK " + targetClient->getNick();
	if (!reason.empty())
		kickMessage += " :" + reason;
	kickMessage += "\r\n";
	//ver se o self precisa receber tambem
	channel->broadcast(NULL, kickMessage);

    // Remove Client from the channel
    channel->removeClientOperator(targetClient->getFd());

	// Confirmação de KICK para o operador
	sendMessageToClient(cl->getFd(), ":42_IRC 403 " + cl->getNick() + " " + cmd[0] + " :KICK successful\r\n");
}

void Server::TOPIC(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 2)
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 461 " + cl->getNick() + " " + cmd[0] + " :Not enough parameters given\r\n");
		return;
	}

	std::string channelName = cmd[1];

	if (channelName[0] != '#')
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	if (!channel->getClientByFd(cl->getFd()))
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 442 " + cl->getNick() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	// Check if mode +t (only ops can set the topic) is enabled
	if (channel->isTopicRestricted() && !channel->isOperator(cl))
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 482 " + cl->getNick() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	if (cmd.size() == 2)
	{
		sendMessageToClient(cl->getFd(), ":42_IRC 332 " + cl->getNick() + " " + channelName + " :" + channel->getTopic() + "\r\n");
		return;
	}

	std::string newTopic;
	if (cmd[2][0] == ':')
	{
		for (size_t i = 2; i < cmd.size(); ++i)
		{
			newTopic += cmd[i];
			if (i != cmd.size() - 1)
				newTopic += " ";
		}
		// Removes : from the beggining
		newTopic.erase(0, 1);
	}
	else
	{
		newTopic = cmd[2];
	}

	channel->setTopic(newTopic);
	sendMessageToClient(cl->getFd(), ":42_IRC 332 " + cl->getNick() + " " + channelName + " :Topic changed to '" + newTopic + "'\r\n");
	channel->broadcast(cl, ":" + cl->getNick() + " TOPIC " + channelName + " :" + newTopic + "\r\n");

	channel->listChannelInfo();
}

Channel* Server::getChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return (it->second);
	return (NULL);
}

// for debugging
void Server::LCI(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 2)
	{
		// No channel specified
		sendMessageToClient(cl->getFd(), "Syntax: LCI #channel{,<#ch1>,<#ch2>,...}\r\n");
		return;
	}

	std::vector<std::string> channels = split(cmd[1], ",");
	std::vector<std::string>::iterator it;
	
	for (it = channels.begin(); it != channels.end(); ++it)
	{
		if ((*it)[0] != '#')
		{
			// Invalid channel name
			sendMessageToClient(cl->getFd(), ":42_IRC 403 " + cl->getNick() + " " + *it + " :No such channel\r\n");
			continue;
		}

		Channel* channel = getChannel(*it);
		if (!channel)
		{
			// Channel does not exist
			sendMessageToClient(cl->getFd(), ":42_IRC 403 " + cl->getNick() + " " + *it + " :No such channel\r\n");
			continue;
		}
		channel->listChannelInfo();
	}
}

void Server::clearChannels()
{
	for(std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
}

bool Server::isChannelExist(std::string channelName)
{
	if (_channels.find(channelName) != _channels.end())
		return (true);
	else
		return (false);
}
