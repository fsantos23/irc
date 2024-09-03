/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 11:35:30 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/03 17:19:20 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

	std::cout << GRE << "Server running and listening on port: <" << _port << WHI << std::endl;
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
				{
                    handleClientMessage(pfd.fd);
				}
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
	/* newClient.setIp(client_addr.sin_addr); */

	Client newClient;
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
	for (std::vector<Client>::iterator it = _cl.begin(); it != _cl.end(); ++it) {
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
	// PFV
	// confirmar o tamanho do buffer (512)
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

	std::vector<std::string> tokens;
	std::istringstream iss(buffer);
	std::string token;

	while (iss >> token)
	{
		tokens.push_back(token);
	}

	// PFV
	// std::cout << "Received message from client (buffer): " << buffer << std::endl;
	// print_tokens(tokens);

	handleInput(tokens, client_fd);
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

	// PFV
	// comentado para testar o envio de mensagens	
//	if(checkIfRegistered(str, &_cl[j]))
//		return ;
		
	mainCommands(str, &_cl[j]);
}

int Server::checkIfRegistered(std::vector<std::string> str, Client *cl)
{
	//handle of main info client
	std::string entry_array[] = {"USER", "NICK", "PASS"};
	std::vector<std::string> entry(entry_array, entry_array + 3);

	std::string user_array[] = {cl->getUser(), cl->getNick(), cl->getPass()};
	std::vector<std::string> user(user_array, user_array + 3);

	for(int i = 0; i < 3; i++)
	{
		if(str[0] == entry[i])
		{
			if(user[i].empty())
			{
				if (i == 0)
					cl->setUser(str[1]);
				else if (i == 1)
					cl->setNick(str[1]);
				else if (i == 2 && str[1] == _password)
				{
					cl->setPass(str[1]);
					sendColoredMessage(cl->getFd(), "Correct password\n", GRE);
					send(cl->getFd(), cl->getPass().c_str(), sizeof(cl->getPass().c_str()) + 1, 0);
				}
				else if (i == 2 && str[1] != _password)
					sendColoredMessage(cl->getFd(), "Wrong password\n", RED);
			}
			if(!user[i].empty())
				send(cl->getFd(), "Alreday set\n", 13, 0);
			return 1;
		}
	}
	
	for(int i = 0; i < 3; i++)
	{
		if(user[i].empty())
		{
			sendColoredMessage(cl->getFd(), "You have to create an account first\n", RED);
			return 1;
		}
	}
	return 0;
}

void Server::mainCommands(std::vector<std::string> command, Client *cl)
{
	(void)cl;

	if (command[0] == "NICK")
		{
			sendMessageToClient(cl->getFd(), "Chama a função NICK...\n");
		}
		else if (command[0] == "PING")
		{
			sendMessageToClient(cl->getFd(), "Chama a função PING...\n");
		}
		else if (command[0] == "QUIT")
		{
			sendMessageToClient(cl->getFd(), "Chama a função QUIT...\n");
		}
		else if (command[0] == "JOIN")
		{
			cmdJoin(command, cl);
		}
		else if (command[0] == "PART")
		{
			sendMessageToClient(cl->getFd(), "Chama a função PART...\n");
		}
		else if (command[0] == "KICK")
		{
			sendMessageToClient(cl->getFd(), "Chama a função KICK...\n");
		}
		else if (command[0] == "PRIVMSG")
		{
			sendMessageToClient(cl->getFd(), "Chama a função PRIVMSG...\n");
		}
		else
		{
			sendMessageToClient(cl->getFd(), "Unknown command\n");
		}
}