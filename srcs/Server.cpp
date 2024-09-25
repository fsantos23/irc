/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:50:46 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/25 11:38:04 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

Server::Server(int port, const std::string password) : _port(port), _password(password), _sockfd(-1)
{
}

Server::~Server()
{
}

void Server::initServer()
{
	serSocket();

	std::cout << GRE << "Server Running and listening on port " << _port << std::endl << "Waiting for connections..." << WHI << std::endl;

	while (true)
	{
		signal(SIGINT, handleSignal);
		signal(SIGTSTP, SIG_IGN);
		
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
	closeChannels();
	closeFds();
}

void Server::serSocket()
{
	//create socket
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (_sockfd == -1)
		throw(std::runtime_error("Socket creation error"));
	int opt = 1;
	//set rule to reuse port
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw(std::runtime_error("Socket reuse rule error"));
	
	struct sockaddr_in server_addr;				//bind type
	server_addr.sin_family = AF_INET;			//IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY;	//bind to any IP address
	server_addr.sin_port = htons(_port);		//bind to port

	if(bind(_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) //bind socket to port
		throw(std::runtime_error("Bind failed"));

	//listen for connection
	if(listen(_sockfd, SOMAXCONN) < 0)
		throw(std::runtime_error("Listen failed"));


	struct pollfd server_pollfd;
	server_pollfd.fd = _sockfd;
	server_pollfd.events = POLLIN;
	server_pollfd.revents = 0;
	_pollfds.push_back(server_pollfd);
}

void Server::closeFds()
{
	for (std::vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it)
	{
		if (it->fd >= 0)
			close(it->fd);
	}
	if (_sockfd >= 0)
	{
		close(_sockfd);
		_sockfd = -1;
	}
}

void Server::handleSignal(int signum)
{
	(void)signum;
	throw(std::runtime_error("Server shuting down"));
}

void Server::closeChannels()
{
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		// free the memory allocated for the channel
		delete it->second;

	// Clean channel map
	_channels.clear();
}

void Server::closeClients()
{
	for (std::vector<Client*>::iterator it = _cl.begin(); it != _cl.end(); ++it)
	{
		close((*it)->getFd());
		delete *it;
	}
	_cl.clear();
}

void Server::acceptNewClient()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_sock = accept(_sockfd, (struct sockaddr *)&client_addr, &client_len); //accept new client
	if(client_sock < 0)
		throw(std::runtime_error("Connecting client fail"));

	Client *newClient = new Client;
	newClient->setIp(inet_ntoa(client_addr.sin_addr));

	newClient->setFd(client_sock);
	// add new client to vector
	_cl.push_back(newClient);

	// MSG to Server Console
	std::cout << GRE << "Client connected: " << inet_ntoa(client_addr.sin_addr) << WHI << std::endl;

	struct pollfd client_pollfd;
	client_pollfd.fd = client_sock;
	client_pollfd.events = POLLIN;
	client_pollfd.revents = 0;
	_pollfds.push_back(client_pollfd);
}

void Server::clearClient(int fd, std::string msg)
{
	// Check if the client is in any channel and remove it
	for (std::map<std::string, Channel*>::iterator chan_it = _channels.begin(); chan_it != _channels.end(); ++chan_it)
	{
		Channel* channel = chan_it->second;
		Client* client = channel->getClientByFd(fd);
		if (client)
		{
			channel->removeClientOperator(fd);
			// MSG to Server Console
			std::cout << GRE << "Client " << fd << " removed from channel " << channel->getChannelName() << WHI << std::endl;

			// Send a message to the channel informing that the client has left
			channel->sendMessageChannel(msg);

			// Check if the channel has no operators and still has clients
			if (channel->countOperators() == 0 && channel->countClients() > 0)
				channel->forceOperator();
			else if (channel->countClients() == 0)
			{
				std::cout << GRE << "Channel " << channel->getChannelName() << " has been removed from the server." << WHI << std::endl;
				channel->clearChannel();
			}
		}
	}
	
	// Remove the client from the Server clients list without deleting it yet
	for (std::vector<Client *>::iterator it = _cl.begin(); it != _cl.end(); ++it)
	{
		if ((*it)->getFd() == fd)
		{
			// Remove the pollfd entry for the client
			for (std::vector<pollfd>::iterator poll_it = _pollfds.begin(); poll_it != _pollfds.end();)
			{
				if (poll_it->fd == fd)
					// Adjust the iterator after erasure
					poll_it = _pollfds.erase(poll_it);
				else
					++poll_it;
			}
			close(fd);
			delete *it;
			_cl.erase(it);
			break;
		}
	}
}

void Server::handleClientMessage(int client_fd)
{
	char buffer[510];
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
	
	_clientBuffers[client_fd] += buffer;

	std::string& clientBuffer = _clientBuffers[client_fd];
	size_t pos;
	
	while ((pos = clientBuffer.find('\n')) != std::string::npos)
	{
		std::string command = clientBuffer.substr(0, pos);
		clientBuffer.erase(0, pos + 1);

		if (!command.empty() && command[command.size() - 1] == '\r')
		{
			command.erase(command.size() - 1);
		}

		if (!command.empty())
		{
			std::vector<std::string> commandParts;
			std::istringstream lineStream(command);
			std::string part;

			while (lineStream >> part)
			{
				commandParts.push_back(part);
			}

			handleInput(commandParts, client_fd);
		}
	}
}

void Server::handleInput(std::vector<std::string> str, int client_fd)
{
	int j = 0;

	for(size_t i = 0; i < _cl.size(); i++)
	{
		if(_cl[i]->getFd() == client_fd)
		{
			j = i;
			break;
		}
	}
	
	if(QUIT(_cl[j], str))
		return ;
	if(checkEntry(str, _cl[j]))
		return ;
	mainCommands(str, _cl[j]);
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
				sendError(cl->getFd(), cl->getNick(), 462, ":You may not reregister");
				return (1);
			}
			return (1);
		}
	}
	for (int i = 0; i < 3; i++)
	{
		if (user[i] == "*" || user[i].empty())
		{
			sendError(cl->getFd(), cl->getNick(), 451, ":You have not registered");
			return (1);
		}
	}
	return (0);
}

void Server::checkNick(std::string str, Client *cl)
{

	std::string lowerNick = toLowerCase(str);
	for (std::vector<Client *>::const_iterator it = _cl.begin(); it != _cl.end(); ++it)
	{
		if (toLowerCase((*it)->getNick()) == lowerNick)
		{
			cl->setTemporaryNick(str);
			sendError(cl->getFd(), cl->getNick(), 433, str + " :Nickname is already in use");
			return ;
		}
	}
	if(!cl->getTemporaryNick().empty())
		sendMessageToClient(cl->getFd(), ":" + cl->getTemporaryNick() + "!" + cl->getUser() + "@localhost NICK :" + str + "\r\n");
	cl->setNick(str);
	// Msg to Server Console
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
	cl->setUser(str[1]);
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
	commandhandler["WHO"] = &Server::WHO;
// for debugging
//	commandhandler["LCI"] = &Server::LCI;

	std::map<std::string, CommandHolder>::iterator it = commandhandler.find(str[0]);
	if(it != commandhandler.end())
		(this->*(it->second))(str, cl);
	else
		sendError(cl->getFd(), cl->getNick(), 421, ":Unknown command");
}

void Server::sendError(int client_fd, const std::string& nickname, int error_code, const std::string& message)
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
		send(_cl[i]->getFd(), msg.c_str(), msg.size(), 0);
}

bool Server::QUIT(Client *cl, std::vector<std::string> str)
{
	if (str[0] == "QUIT")
	{
		// Msg to Server Console 
		std::cout << cl->getNick() << " has quit" << std::endl;

		std::string quitMessage = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " QUIT ";
		if (str.size() > 1)
		{
			for (size_t i = 1; i < str.size(); ++i)
				quitMessage += str[i] + " ";

			quitMessage += "\r\n";
		} 
		else
			quitMessage += ":Client Quit\r\n";

		clearClient(cl->getFd(), quitMessage);
	
		return (1);
	}
	return (0);
}

void Server::PRIVMSG(std::vector<std::string> str, Client *cl)
{
    std::string sender = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " PRIVMSG ";
    if (str.size() < 3) 
    {
        sendError(cl->getFd(), cl->getNick(), 411, ":No recipient given");
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
			if (it->first == str[1] && it->second->getClientByName(cl->getNick()))
			{
				std::string message = sender + str[1] + " " + fullMessage + "\r\n";
				it->second->broadcast(cl, message);
				recipientFound = true;
			}
			else if(it->first == str[1] && !it->second->getClientByName(cl->getNick()))
			{
				sendError(cl->getFd(), cl->getNick(), 442, str[1] + " :You're not on that channel");
				recipientFound = true;
				break;
			}
		}
    }
	else
	{
		for (size_t i = 0; i < _cl.size(); ++i)
		{
			if (str[1] == _cl[i]->getNick())
			{
				std::string message = sender + str[1] + " " + fullMessage + "\r\n";
				send(_cl[i]->getFd(), message.c_str(), message.length(), 0);
				recipientFound = true;
				break;
			}
		}
	}
	if (!recipientFound)
		sendError(cl->getFd(), cl->getNick(), 401, ":No such nick/channel");
}

void Server::JOIN(std::vector<std::string> cmd, Client *cl)
{
	
	if (cmd.size() < 2)
	{
		sendError(cl->getFd(), cl->getNick(), 461, " JOIN :Not enough parameters");
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
			sendError(cl->getFd(), cl->getNick(), 403, *it + " :No such channel");
			continue;
		}

		// Join or create the channel.
		Channel* channel = joinChannel(*it, cl);

		if(channel->countClients() == 0 && channel->countOperators() == 0)
		{
			channel->addOperator(cl);
			std::cout << GRE << "Client " << cl->getFd() << " is the operator of channel " << channel->getChannelName() << WHI << std::endl;
		}
		// Check if the channel is invite-only and if the client is allowed to join.
		if (channel->isInviteOnly() && !channel->isInvited(cl))
		{
			sendError(cl->getFd(), cl->getNick(), 473, *it + " :Cannot join channel (+i)");
			continue;
		}
		//check if the channel has a key and if the client has the key
		if(channel->hasKey() && key_it == keypass.end())
		{
			sendError(cl->getFd(), cl->getNick(), 475, *it + " :Cannot join channel (+k)");
			continue;
		}
		// Validate the channel key if necessary.
		if (channel->hasKey() && !channel->checkKey(*key_it))
		{
			sendError(cl->getFd(), cl->getNick(), 475, *it + " :Cannot join channel (+k)");
			continue;
		}

		// Check if the client is already in the channel.
		if (!channel->isNewClient(cl->getFd()))
			continue;

		// Check if the channel has a user limit and if it has been reached.
		if (channel->hasUserLimit() && channel->countClients() >= channel->getUserLimit())
		{
			sendError(cl->getFd(), cl->getNick(), 471, *it + " :Cannot join channel (+l)");
			continue;
		}
		
		// Add the client to the channel.
		channel->addClient(cl);
		std::cout << GRE << "Client " << cl->getFd() << " joined channel " << *it << WHI << std::endl;

		std::string joinMessage = ":" + cl->getNick() + "!" + cl->getUser() + "@localhost" + " " + cmd[0] + " " + *it + " * :realname""\r\n";
		sendMessageToClient(cl->getFd(), joinMessage);
		channel->broadcast(cl, joinMessage);

		if (key_it != keypass.end())
			++key_it;
	}
}

Channel* Server::joinChannel(const std::string& name, Client *cl)
{
	if (isChannelExist(name))
		return (_channels[name]);

	// If the channel does not exist, creates a new one.
	Channel* newChannel = new Channel(name);
	_channels[name] = newChannel;
	// Server Console MSG
	std::cout << GRE << "Client " << cl->getFd() << " created channel " << name << WHI << std::endl;

	// Add the client as the operator of the newly created channel
	newChannel->addOperator(cl);
	// Server Console MSG
	std::cout << GRE << "Client " << cl->getFd() << " is the operator of channel " << name << WHI << std::endl;
	
	return (newChannel);
}

void Server::PART(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 2)
	{
		// Not enough parameters
		sendError(cl->getFd(), cl->getNick(), 461, "PART :Not enough parameters");
		return;
	}

	std::vector<std::string> channels = split(cmd[1], ",");
	std::vector<std::string>::iterator it;

	for (it = channels.begin(); it != channels.end(); ++it)
	{
		if ((*it)[0] != '#')
		{
			// Invalid channel name
			sendError(cl->getFd(), cl->getNick(), 403, *it + " :No such channel");
			continue;
		}

		Channel* channel = getChannel(*it);
		if (!channel)
		{
			// Channel does not exist
			sendError(cl->getFd(), cl->getNick(), 403, *it + " :No such channel");
			continue;
		}

		if (channel->isNewClient(cl->getFd()))
		{
			std::cout << YEL << "Client : " << cl->getFd() << " is not on the channel" << WHI << std::endl;
			sendError(cl->getFd(), cl->getNick(), 442, *it + " :You're not on that channel");
			continue;
		}

		// Broadcast a message to the channel notifying other users that the client has left
		std::string message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + *it;

		message += "\r\n";
		channel->sendMessageChannel(message);

		// Remove the client from the channel
		channel->removeClientOperator(cl->getFd());
		std::cout << GRE << "Client : " << cl->getFd() << " left the channel " << channel->getChannelName() << WHI << std::endl;

		if (channel->countOperators() == 0 && channel->countClients() > 0)
		{
			channel->forceOperator();
		}
		else if (channel->countClients() == 0)
		{
			channel->clearChannel();
			std::cout << GRE << "Channel " << channel->getChannelName() << " has been removed from the server." << WHI << std::endl;
		}
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
				int aux = 0;
				std::string msga = cmd[1] + " ";
				std::string msgb = "";
				std::string info = " ";
				if(it->second->getUserLimit())
				{
					msgb += "l";
					aux = 1;
					std::stringstream ss;
					ss << it->second->getUserLimit();  // Convert int to string
					info += ss.str() + " ";
				}
				if(!it->second->getKey().empty())
				{
					msgb += "k";
					aux = 1;
					if(it->second->isOperator(cl))
						info += it->second->getKey() + " ";
					else
						info += '*';
				}
				if(!it->second->getTopic().empty())
				{
					msgb += "t";
					aux = 1;
				}
				if (aux == 1)
					msga += "+" + msgb;
				msga += info;
 				sendError(cl->getFd(), cl->getNick(), 324, msga);
				return;
			}
		}
	}

	if (cmd.size() < 3)
	{
		sendError(cl->getFd(), cl->getNick(), 461, cmd[0] + " :Not enough parameters given");
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
		sendError(cl->getFd(), cl->getNick(), 403, channelName + " :No such channel");
		return;
	}

	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendError(cl->getFd(), cl->getNick(), 403, channelName + " :No such channel");
		return;
	}

	// Check if the client has operator permissions in this channel.
	if (!channel->isOperator(cl))
	{
		sendError(cl->getFd(), cl->getNick(), 482, channelName + " :You're not channel operator");
		return;
	}

	std::string message;
	// Handle invite-only mode (+i/-i)
	if (mode == "+i")
	{
		channel->setInviteOnly(true);
		message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + " " + mode + "\r\n";
		channel->sendMessageChannel(message);
	}
	else if (mode == "-i")
	{
		channel->setInviteOnly(false);
		message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + " " + mode + "\r\n";
		channel->sendMessageChannel(message);
	}
	// Handle key mode (+k/-k)
	else if (mode == "+k")
	{
		if (key_nick_or_limit.empty())
		{
			sendError(cl->getFd(), cl->getNick(), 461, cmd[0] + mode + " :Not enough parameters\r\n");
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
			sendError(cl->getFd(), cl->getNick(), 461, channelName + " " + mode + " :Not enough parameters");
			return;
		}
		if (!isdigit(key_nick_or_limit[0]))
		{
			sendError(cl->getFd(), cl->getNick(), 461, channelName + " " + mode + " :Not a valid limit");
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
			sendError(cl->getFd(), cl->getNick(), 401, key_nick_or_limit + " :No such nick\r\n");
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
			sendError(cl->getFd(), cl->getNick(), 401, key_nick_or_limit + " :No such nick\r\n");
	}
	// Handle topic restriction mode (+t/-t)
	else if (mode == "+t")
	{
		channel->setTopicRestricted(true);
		message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + " " + mode + " " + key_nick_or_limit + "\r\n";
		channel->sendMessageChannel(message);
	}
	else if (mode == "-t")
	{
		channel->setTopicRestricted(false);
		message = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + channelName + " " + mode + " " + key_nick_or_limit + "\r\n";
		channel->sendMessageChannel(message);
	}
	else
		sendError(cl->getFd(), cl->getNick(), 472, mode + " :is unknown mode char to me");
}

void Server::INVITE(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 3)
	{
		// Not enough parameters
		sendError(cl->getFd(), cl->getNick(), 461, cmd[0] + " :Not enough parameters given");
		return;
	}

	// Extract nickname and channel name
	std::string invitedNick = cmd[1];
	std::string channelName = cmd[2];

	// Checks if the channel name is valid (must start with '#')
	if (channelName[0] != '#')
	{
		sendError(cl->getFd(), cl->getNick(), 403, channelName + " :No such channel");
		return;
	}

	// Checks if the channel exists
	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendError(cl->getFd(), cl->getNick(), 403, channelName + " :No such channel");
		return;
	}

	// Checks if the client is an operator in the channel
	if (!channel->isOperator(cl))
	{
		sendError(cl->getFd(), cl->getNick(), 482, channelName + " :You're not channel operator");
		return;
	}

	// Checks if the client to be invited exists
	Client* invitedClient = NULL;
	for (std::vector<Client *>::iterator it = _cl.begin(); it != _cl.end(); ++it)
	{
		if ((*it)->getNick() == invitedNick)
		{
			invitedClient = (*it);
			break;
		}
	}
	
	if (!invitedClient)
	{
		sendError(cl->getFd(), cl->getNick(), 401, invitedNick + " :No such nick");
		return;
	}

	// Checks if the client is already in the channel
	if (!channel->isNewClient(invitedClient->getFd()))
	{
		sendError(cl->getFd(), cl->getNick(), 443, invitedNick + " " + channelName + " :is already on channel");
		return;
	}

	// Add the client to the channel invited clients list
	channel->inviteClient(invitedClient);

	std::string inviteMessage = ":42_IRC 341 " + cl->getNick() + " " + invitedClient->getNick() + " " + channelName + "\r\n";
	sendMessageToClient(cl->getFd(), inviteMessage);
	std::string confirmationMessage = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " " + cmd[0] + " " + invitedClient->getNick() + " " + channelName + "\r\n";
	sendMessageToClient(invitedClient->getFd(), confirmationMessage);
}

void Server::KICK(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 3)
	{
		// Not enough parameters
		sendError(cl->getFd(), cl->getNick(), 461, cmd[0] + " :Not enough parameters given");
		return;
	}

	std::vector<std::string> channel_get = split(cmd[1], ",");
	std::vector<std::string> target = split(cmd[2], ",");
	std::string reason;

	std::vector<std::string>::iterator targetNick = target.begin();

	for(std::vector<std::string>::iterator channelName = channel_get.begin(); channelName != channel_get.end(); ++channelName)
	{
		for (size_t i = 3; i < cmd.size(); ++i)
		{
			reason += cmd[i] + " ";
		}
		
		// Checks if the channel exists
		Channel* channel = getChannel(*channelName);
		if (!channel)
		{
			sendError(cl->getFd(), cl->getNick(), 403, *channelName + " :No such channel");
			return;
		}

		// Check if Client is in the channel
		Client* targetClient = NULL;
		targetClient = channel->getClientByName(*targetNick);

		if (!targetClient)
		{
			sendError(cl->getFd(), cl->getNick(), 401, *targetNick + " :No such nick");
			return;
		}

		// Checks if the client is an operator in the channel
		if (!channel->isOperator(cl))
		{
			sendError(cl->getFd(), cl->getNick(), 482, *channelName + " :You're not channel operator");
			return;
		}

		// Sends a message to all clients in the channel, informing about the KICK
		std::string kickMessage = ":" + cl->getNick() + " " + cmd[0] + " " + *channelName + " " + targetClient->getNick();

		if (!reason.empty())
			kickMessage += " " + reason;
		kickMessage += "\r\n";

		channel->sendMessageChannel(kickMessage);
		// Remove Client from the channel
		channel->removeClientOperator(targetClient->getFd());
		// Remove Client from the invited list
		channel->removeInvited(targetClient->getFd());

		if (targetNick != target.end())
			++targetNick;

		reason = "";
	}
}

void Server::TOPIC(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 2)
	{
		sendError(cl->getFd(), cl->getNick(), 461, cmd[0] + " :Not enough parameters given");
		return;
	}

	std::string channelName = cmd[1];
	Channel* channel = getChannel(channelName);

	if (cmd.size() == 2)
	{
		if (channel)
			sendMessageToClient(cl->getFd(), ":42_IRC 332 " + cl->getNick() + " " + channelName + " :" + channel->getTopic() + "\r\n");
		else
			sendError(cl->getFd(), cl->getNick(), 403, channelName + " :No such channel");
		return;
	}

	if (channelName[0] != '#')
	{
		sendError(cl->getFd(), cl->getNick(), 403, channelName + " :No such channel");
		return;
	}

	if (!channel)
	{
		sendError(cl->getFd(), cl->getNick(), 403, channelName + " :No such channel");
		return;
	}

	// Check if mode +t is enabled (only operators can set the topic)
	if (channel->isTopicRestricted() && !channel->isOperator(cl))
	{
		sendError(cl->getFd(), cl->getNick(), 482, channelName + " :You're not channel operator");
		return;
	}

	if (!channel->getClientByFd(cl->getFd()))
	{
		sendError(cl->getFd(), cl->getNick(), 442, channelName + " :You're not on that channel");
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
	sendMessageToClient(cl->getFd(), ":42_IRC 332 " + cl->getNick() + " " + channelName + " :" + newTopic + "\r\n");
	channel->broadcast(cl, ":" + cl->getNick() + " TOPIC " + channelName + " :" + newTopic + "\r\n");
}

void Server::WHO(std::vector<std::string> cmd, Client* cl)
{
	if(cmd[1][0] == '#')
	{
		Channel *channel = getChannel(cmd[1]);
		if(!channel)
		{
			sendError(cl->getFd(), cl->getNick(), 403, cmd[1] + " :No such channel");
			return;
		}
		
		std::string nameList = channel->getClientList();
		std::string nameReply = ":42_IRC 353 " + cl->getNick() + " = " + channel->getChannelName() + " :" + nameList + "\r\n";
		sendMessageToClient(cl->getFd(), nameReply);

		std::string endOfNamesReply = ":42_IRC 366 " + cl->getNick() + " " + channel->getChannelName() + " :End of /NAMES list\r\n";
		sendMessageToClient(cl->getFd(), endOfNamesReply);
	}
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
			sendError(cl->getFd(), cl->getNick(), 403, *it + " :No such channel");
			continue;
		}
	}
}

Channel* Server::getChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return (it->second);
	return (NULL);
}

bool Server::isChannelExist(std::string channelName)
{
	if (_channels.find(channelName) != _channels.end())
		return (true);
	else
		return (false);
}
