/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: paulo <paulo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:52:17 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/13 11:27:45 by paulo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

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
#include <sstream>

class Client {
	private:
		int _fd;
		std::string _ip;
		std::string _nickname;
		std::string _user;
		std::string _pass;
		std::string _temporaryNick;

	public:
		Client();
		int getFd();
		void setFd(int fd);
		void setIp(std::string ip);
		void setTemporaryNick(std::string str) {_temporaryNick = str;};
		std::string getIp() const {return _ip;};
		std::string getNick() const {return _nickname;};
		std::string getUser() const {return _user;};
		std::string getPass() const {return _pass;};
		std::string getTemporaryNick() {return _temporaryNick;};
		void setNick(std::string str) {_nickname = str;};
		void setUser(std::string str) {_user = str;}
		void setPass(std::string str) {_pass = str;}
};

#endif