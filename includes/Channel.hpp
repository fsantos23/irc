/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 17:20:16 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/03 17:20:19 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Server.hpp"

class Channel
{
	private:
		std::string _name;
		std::vector<int> _sockcl;

	public:
		Channel(std::string _name);
		~Channel();

		void createChannel(Client *cl);
		void addClient(Client *cl);
		void clearClient(int cl_fd);
		void broadcast(Client* self, const std::string &msg);
};

#endif