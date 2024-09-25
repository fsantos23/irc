/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/12 10:53:54 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/25 10:33:05 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include "Server.hpp"

void	print_tokens(std::vector<std::string> str);
void	sendMessageToClient(int client_fd, const std::string& message);
std::vector<std::string>	split(std::string str, std::string delimiter);
std::string toLowerCase(const std::string& str);

#endif