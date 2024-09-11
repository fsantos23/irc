#include "../includes/Server.hpp"

Server::Server(int port, const std::string password) : _port(port), _password(password), _sockfd(-1), _signal(false), _sockcl(0)
{

}

Server::~Server()
{
	closeFds(); //close all fds after server closes
}

void Server::initServer()
{
	serSocket();

	std::cout << GRE << "Server Running and listening on port " << _port << std::endl << "Waiting for connections..." << WHI << std::endl;
	_signal = true;
	while (true)
	{
		signal(SIGINT, handleSignal);
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

void Server::handleSignal(int signum)
{
	if(signum == SIGINT)
		throw(std::runtime_error("Server shuting down"));
}

void Server::acceptNewClient()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_sock = accept(_sockfd, (struct sockaddr *)&client_addr, &client_len); //accept new client
	if(client_sock < 0)
		throw(std::runtime_error("Connecting client fail"));

	Client newClient;
	newClient.setIp(inet_ntoa(client_addr.sin_addr));
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

void Server::clearClient(int fd)
{
	for (std::vector<Client>::iterator it = _cl.begin(); it != _cl.end(); ++it)
	{
        if (it->getFd() == fd) 
		{
            close(fd);  // Close the client socket
            _cl.erase(it);  // Remove from the client list

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
		clearClient(client_fd);
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
	checkQuit(_cl[j], str);
	if(checkEntry(str, &_cl[j]))
		return ;
	mainCommands(str, &_cl[j]);
}

int Server::checkEntry(std::vector<std::string> str, Client *cl)
{
	//o user e nick tem de ser diferentes em todos os users
	std::string entry_array[] = {"USER", "NICK", "PASS"};
    std::vector<std::string> entry(entry_array, entry_array + 3);

    std::string user_array[] = {cl->getUser(), cl->getNick(), cl->getPass()};
    std::vector<std::string> user(user_array, user_array + 3);

    for (int i = 0; i < 3; i++)
    {
        if (str[0] == entry[i])
        {
			if (i == 1)
                cl->setNick(str[1]);
            else if (user[i] == "*" && i == 0)
                cl->setUser(str[1]);
            else if (i == 2 && str[1] == _password && user[i].empty())
                cl->setPass(str[1]);
            else if (i == 2 && str[1] != _password && user[i].empty())
                return 1;
            else
            {
                sendError(cl->getFd(), cl->getNick(), 462, "You may not reregister");
                return 1;
            }
			return 1;
        }
    }
    for (int i = 0; i < 3; i++)
    {
        if (user[i] == "*" || user[i].empty())
        {
            sendError(cl->getFd(), cl->getNick(), 451, "You have not registered");
            return 1;
        }
    }
    return 0;
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
	// PFV
	commandhandler["LISTINFO"] = &Server::LISTINFO;

	std::map<std::string, CommandHolder>::iterator it = commandhandler.find(str[0]);
	if(it != commandhandler.end())
		(this->*(it->second))(str, cl);
	else
		send(cl->getFd(), "Command doesn't exist\r\n", 23, 0);
}

void Server::sendError(int client_fd, const std::string& nickname, int error_code, const std::string& message)
{
    std::string server_name = "42_IRC";
    std::ostringstream oss;
    oss << ":" << server_name << " " << error_code << " " << nickname << " :" << message << "\r\n";
    std::string error_message = oss.str();
    send(client_fd, error_message.c_str(), error_message.length(), 0);
}

void Server::sendMessageAll(std::string msg)
{
	msg += "\r\n";
	for(size_t i = 0; i < _cl.size(); i++)
		send(_cl[i].getFd(), msg.c_str(), msg.size(), 0);
}

void Server::checkQuit(Client cl, std::vector<std::string> str)
{
	if (str[0] == "QUIT") 
	{
		clearClient(cl.getFd());
		std::string quitMessage;
		if (str.size() > 1)
		{
			for (size_t i = 1; i < str.size(); ++i)
				quitMessage += str[i] + " ";
			if (quitMessage[0] == ':')
            	quitMessage = quitMessage.substr(1);
			quitMessage = quitMessage.substr(0, quitMessage.size() - 1);
			sendMessageAll(cl.getNick() + " has quit (" + quitMessage + ")");
		} 
		else
			sendMessageAll(cl.getNick() + " has quit (Client disconnected)");
	}
}

void Server::PRIVMSG(std::vector<std::string> str, Client *cl)
{
    std::string sender = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " PRIVMSG ";
    if (str.size() < 3) 
    {
        sendError(cl->getFd(), cl->getNick(), 411, "No recipient given");
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
				it->second->broadcast(cl, message);  // it->second is the Channel* object
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
		sendError(cl->getFd(), cl->getNick(), 401, "No such nick/channel");
}


void Server::JOIN(std::vector<std::string> cmd, Client *cl)
{
	// Check if there are enough parameters for the JOIN command.
	if (cmd.size() < 2)
	{
		sendError(cl->getFd(), cl->getNick(), 461, "JOIN :Not enough parameters");
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

		// Check if the channel is invite-only and if the client is allowed to join.
		if (channel->isInviteOnly() && !channel->isInvited(cl))
		{
			sendError(cl->getFd(), cl->getNick(), 473, *it + " :Cannot join channel (invite only)");
			continue;
		}

		// Validate the channel key if necessary.
		if (channel->hasKey() && !channel->checkKey(*key_it))
		{
			sendError(cl->getFd(), cl->getNick(), 475, *it + " :Cannot join channel (incorrect key)");
			continue;
		}

		// Check if the client is already in the channel.
		if (!channel->isNewClient(cl->getFd()))
		{
			sendError(cl->getFd(), cl->getNick(), 443, *it + " :You're already in the channel");
			continue;
		}

		// Add the client to the channel.
		channel->addClient(cl->getFd(), cl);

		// 1. Broadcast the JOIN message to the channel.
		std::string joinMessage = ":" + cl->getNick() + "!" + cl->getUser() + "@" + cl->getIp() + " JOIN :" + *it + "\r\n";
		channel->sendMessageChannel(joinMessage);

        // 2. Send RPL_NAMREPLY (353) to the client, showing the list of users in the channel.
        /* std::string nameList = channel->getClientList(); */ // Assuming this function exists and returns the list of users.
        /* std::string nameReply = ":42_IRC 353 " + cl->getNick() + " = " + *it + " :" + nameList + "\r\n";
        sendMessageToClient(cl->getFd(), nameReply); */

		// 3. Send RPL_ENDOFNAMES (366) to indicate the end of the user list.
		std::string endOfNamesReply = ":42_IRC 366 " + cl->getNick() + " " + *it + " :End of /NAMES list\r\n";
		sendMessageToClient(cl->getFd(), endOfNamesReply);

		if (key_it != keypass.end())
			++key_it;

		//PFV
		channel->listChannelInfo();
	}
}

Channel* Server::joinChannel(const std::string& name, Client *cl)
{
	if (isChannelExist(name))
		return (_channels[name]);

	// If the channel does not exist, creates a new one.
	Channel* newChannel = new Channel(name);
	_channels[name] = newChannel;

	// Add the client as the operator of the newly created channel
	newChannel->addOperator(cl->getFd(), cl);

	// Send a message to the client informing them that they are the operator
	std::string message = ":42_IRC 324 " + cl->getNick() + " " + name + " +o\r\n";  // +o gives operator privileges
	sendMessageToClient(cl->getFd(), message);
	message = ":42_IRC 331 "  + cl->getNick() + " " + name + " :No topic is set\r\n";
	sendMessageToClient(cl->getFd(), message);

	return (newChannel);
}

bool Server::isChannelExist(std::string channelName)
{
	if (_channels.find(channelName) != _channels.end())
		return (true);
	else
		return (false);
}

void Server::PART(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 2)
	{
		// Not enough parameters
		sendMessageToClient(cl->getFd(), ":ft_irc.42 4601 " + cl->getNick() + " " + cmd[0] + " :Not enough parameters given\r\n");
		return;
	}

	std::vector<std::string> channels = split(cmd[1], ",");
	std::vector<std::string>::iterator it;

	for (it = channels.begin(); it != channels.end(); ++it)
	{
		if ((*it)[0] != '#')
		{
			// Invalid channel name
			sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + *it + " :No such channel\r\n");
			continue;
		}

		Channel* channel = getChannel(*it);
		if (!channel)
		{
			// Channel does not exist
			sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + *it + " :No such channel\r\n");
			continue;
		}

		if (channel->isNewClient(cl->getFd()))
		{
			// Client is not on the channel
			std::cout << "Client : " << cl->getFd() << " is not on the channel" << std::endl;
			sendMessageToClient(cl->getFd(), ":ft_irc.42 401 " + cl->getNick() + " " + *it + " :You are not on that channel\r\n");
			continue;
		}

		// Remove the client from the channel
		channel->clearClient(cl->getFd());

		if (channel->countOperators() == 0 && channel->countClients() > 0)
			channel->forceOperator();

		// Broadcast a message to the channel notifying other users that the client has left
		std::string message = ":ft_irc.42 332 " + cl->getNick() + " " + *it + " :Leaving " + *it + "\r\n";
		channel->broadcast(cl, message);

		// PFV
		channel->listChannelInfo();
	}
}

Channel* Server::getChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return (it->second);
	return (NULL);
}

void Server::MODE(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 3)
	{
		// Not enough parameters
		sendMessageToClient(cl->getFd(), ":ft_irc.42 461 " + cl->getNick() + " " + cmd[0] + " :Not enough parameters given\r\n");
		return;
	}

	// Extract channel name and mode
	std::string channelName = cmd[1];
	std::string mode = cmd[2];
	// Extract password if exist
	std::string key_nick = (cmd.size() > 3) ? cmd[3] : "";

	// Checks if the channel name is valid (must start with '#')
	if (channelName[0] != '#')
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Checks if the client has operator permissions in this channel.
	if (!channel->isOperator(cl))
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 482 " + cl->getNick() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	// Checks if the mode is being set to "invite-only"
	if (mode == "+i")
	{
		channel->setInviteOnly(true);
		sendMessageToClient(cl->getFd(), ":ft_irc.42 324 " + cl->getNick() + " " + channelName + " :Invite-only mode enabled\r\n");
	}
	else if (mode == "-i")
	{
		channel->setInviteOnly(false);
		sendMessageToClient(cl->getFd(), ":ft_irc.42 324 " + cl->getNick() + " " + channelName + " :Invite-only mode disabled\r\n");
	}
	// Checks if the mode is "key" protected
	else if (mode == "+k")
	{
		if (key_nick.empty())
		{
			sendMessageToClient(cl->getFd(), ":ft_irc.42 324 " + cl->getNick() + " " + cmd[0] + " :Key required.\r\n");
			return;
		}
		channel->setKey(key_nick);
		// PFV
		// channel->broadcast(cl, "Channel key set to: " + key_nick + "\r\n");
		sendMessageAll("Channel key set to: " + key_nick + "\r\n");
	}
	else if (mode == "-k")
	{
		channel->removeKey();
		// PFV
		// channel->broadcast(cl, "Channel key removed.\r\n");
		sendMessageAll("Channel key removed.\r\n");
	}
	// Checks if the mode is Give/take channel "operator privilege"
	if (mode == "+o")
	{
		Client* client = channel->getClientByName(key_nick);
		if (client)
		{
			channel->addOperator(client->getFd(), client);
			sendMessageToClient(cl->getFd(), ":ft_irc.42 324 " + key_nick + " " + channelName + " :Added as Operator\r\n");
		}
		else
		{
			sendMessageToClient(cl->getFd(), ":ft_irc.42 324 " + key_nick + " NOT FOUND in " + channelName + " :NOT FOUND\r\n");
		}
	}
	else if (mode == "-o")
	{
		Client* client = channel->getClientByName(key_nick);
		if (client)
		{
			channel->removeOperator(client->getFd());
			sendMessageToClient(cl->getFd(), ":ft_irc.42 324 " + key_nick + " " + channelName + " :Removed as Operator\r\n");
		}
		else
		{
			sendMessageToClient(cl->getFd(), ":ft_irc.42 324 " + key_nick + " NOT FOUND in " + channelName + " :NOT FOUND\r\n");
		}
	}
	// Checks if the Topic command is restricted to Operators
	else if (mode == "+t")
	{
		channel->setTopicRestricted(true);
		sendMessageToClient(cl->getFd(), ":ft_irc.42 324 " + cl->getNick() + " " + channelName + " :Topic change restricted to operators\r\n");
	}
	// Checks if the Topic command is NOT restricted to Operators
	else if (mode == "-t")
	{
		channel->setTopicRestricted(false);
		sendMessageToClient(cl->getFd(), ":ft_irc.42 324 " + cl->getNick() + " " + channelName + " :Anyone can change the topic\r\n");
	}
	else
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 472 " + cl->getNick() + " " + mode + " :Unknown mode\r\n");
	}

	// PFV
	channel->listChannelInfo();
}

void Server::INVITE(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 3)
	{
		// Not enough parameters
		sendMessageToClient(cl->getFd(), ":ft_irc.42 461 " + cl->getNick() + " " + cmd[0] + " :Not enough parameters given\r\n");
		return;
	}

	// Extract nickname and channel name
	std::string clientNick = cmd[1];
	std::string channelName = cmd[2];

	// Checks if the channel name is valid (must start with '#')
	if (channelName[0] != '#')
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Checks if the channel exists
	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Checks if the client is an operator in the channel
	if (!channel->isOperator(cl))
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 482 " + cl->getNick() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	// Checks if the client to be invited exists
	Client* invitedClient = NULL;
	for (std::vector<Client>::iterator it = _cl.begin(); it != _cl.end(); ++it)
	{
		if (it->getNick() == clientNick)
		{
			invitedClient = &(*it);
			break;
		}
	}
	
	if (!invitedClient)
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 401 " + cl->getNick() + " " + clientNick + " :No such nick\r\n");
		return;
	}

	// Checks if the client is already in the channel
	if (!channel->isNewClient(invitedClient->getFd()))
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 443 " + cl->getNick() + " " + channelName + " :Client is already in the channel\r\n");
		return;
	}

	// Add the client to the channel invited clients list
	channel->inviteClient(invitedClient);

	std::string inviteMessage = ":ft_irc.42 " + cl->getNick() + " INVITE " + invitedClient->getNick() + " :" + channelName + "\r\n";
	sendMessageToClient(invitedClient->getFd(), inviteMessage);

	std::string confirmationMessage = ":ft_irc.42 " + cl->getNick() + " " + cmd[0] + " " + clientNick + " :" + channelName + "\r\n";
	sendMessageToClient(cl->getFd(), confirmationMessage);

	// PFV
	// atualizar ListChannelInfo() com a lista de clientes convidados
	channel->listChannelInfo();
}

void Server::KICK(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 3)
	{
		// Not enough parameters
		sendMessageToClient(cl->getFd(), ":ft_irc.42 461 " + cl->getNick() + " " + cmd[0] + " :Not enough parameters given\r\n");
		return;
	}

	std::string channelName = cmd[1];
	std::string targetNick = cmd[2];
	std::string reason = (cmd.size() > 3) ? cmd[3] : "";

	// Checks if the channel exists
	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
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
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + channelName + " :No such nick in channel\r\n");
		return;
	}

	// Checks if the client is an operator in the channel
	if (!channel->isOperator(cl))
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + cmd[0] + " :You are not an operator in this channel.\r\n");
		return;
	}

	// Sends a message to all clients in the channel, informing about the KICK
	std::string kickMessage = ":" + cl->getNick() + " KICK " + targetClient->getNick();
	if (!reason.empty())
		kickMessage += " :" + reason;
	kickMessage += "\r\n";

	//ver se o self precisa receber tambem
	channel->broadcast(cl, kickMessage);

    // Remove Client from the channel
    channel->removeClient(targetClient->getFd());

	// Confirmação de KICK para o operador
	sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + cmd[0] + " :KICK successful\r\n");
}

// PFV
void Server::LISTINFO(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 2)
	{
		// No channel specified
		sendMessageToClient(cl->getFd(), ":ft_irc.42 461 " + cl->getNick() + " " + cmd[0] + " :Not enough parameters given\r\n");
		return;
	}

	std::vector<std::string> channels = split(cmd[1], ",");
	std::vector<std::string>::iterator it;
	
	for (it = channels.begin(); it != channels.end(); ++it)
	{
		if ((*it)[0] != '#')
		{
			// Invalid channel name
			sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + *it + " :No such channel\r\n");
			continue;
		}

		Channel* channel = getChannel(*it);
		if (!channel)
		{
			// Channel does not exist
			sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + *it + " :No such channel\r\n");
			continue;
		}
		channel->listChannelInfo();
	}
}

/*
void Server::TOPIC(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 2)
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 461 " + cl->getNick() + " " + cmd[0] + " :Not enough parameters given\r\n");
		return;
	}

	std::string channelName = cmd[1];
	std::string newTopic = (cmd.size() > 2) ? cmd[2] : "";

	if (channelName[0] != '#')
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}


	if (!channel->getClientByFd(cl->getFd()))
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 442 " + cl->getNick() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

		// Check if mode +t (only ops can set the topic) is enabled
	if (channel->isTopicRestricted() && !channel->isOperator(cl))
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 482 " + cl->getNick() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	// If no new topic is provided, just send the current topic
	if (newTopic.empty())
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 332 " + cl->getNick() + " " + channelName + " :" + channel->getTopic() + "\r\n");
		return;
	}

	// Define o novo tópico e notifica o canal
	channel->setTopic(newTopic);
	sendMessageToClient(cl->getFd(), ":ft_irc.42 332 " + cl->getNick() + " " + channelName + " :Topic changed to '" + newTopic + "'\r\n");
	channel->broadcast(cl, ":" + cl->getNick() + " TOPIC " + channelName + " :" + newTopic + "\r\n");

	// PFV
	channel->listChannelInfo();
}
*/

void Server::TOPIC(std::vector<std::string> cmd, Client* cl)
{
	if (cmd.size() < 2)
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 461 " + cl->getNick() + " " + cmd[0] + " :Not enough parameters given\r\n");
		return;
	}

	std::string channelName = cmd[1];

	if (channelName[0] != '#')
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 403 " + cl->getNick() + " " + channelName + " :No such channel\r\n");
		return;
	}

	if (!channel->getClientByFd(cl->getFd()))
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 442 " + cl->getNick() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	// Check if mode +t (only ops can set the topic) is enabled
	if (channel->isTopicRestricted() && !channel->isOperator(cl))
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 482 " + cl->getNick() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	if (cmd.size() == 2)
	{
		sendMessageToClient(cl->getFd(), ":ft_irc.42 332 " + cl->getNick() + " " + channelName + " :" + channel->getTopic() + "\r\n");
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
	sendMessageToClient(cl->getFd(), ":ft_irc.42 332 " + cl->getNick() + " " + channelName + " :Topic changed to '" + newTopic + "'\r\n");
	channel->broadcast(cl, ":" + cl->getNick() + " TOPIC " + channelName + " :" + newTopic + "\r\n");

	// PFV
	channel->listChannelInfo();
}
