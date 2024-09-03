/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/02 15:15:17 by pviegas           #+#    #+#             */
/*   Updated: 2024/09/03 15:32:25 by pviegas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

// Error Replies
#define ERR_NOSUCHNICK "401"       // "<nickname> :No such nick/channel" - Used to indicate the nickname parameter supplied to a command is currently unused.
#define ERR_NOSUCHCHANNEL "403"    // "<channel name> :No such channel" - Used to indicate the given channel name is invalid.
#define ERR_CANNOTSENDTOCHAN "404"  // "<channel name> :Cannot send to channel" - Sent to a user who is either (a) not on a channel which is mode +n or (b) not a chanop (or mode +v) on a channel which has mode +m set and is trying to send a PRIVMSG message to that channel.
#define ERR_TOOMANYCHANNELS "405"  // "<channel name> :You have joined too many channels" - Sent to a user when they have joined the maximum number of allowed channels and they try to join another channel.
#define ERR_TOOMANYTARGETS "407"   // "<target> :Duplicate recipients. No message delivered" - Returned to a client which is attempting to send a PRIVMSG/NOTICE using the user@host destination format and for a user@host which has several occurrences.

#define ERR_NEEDMOREPARAMS "461"   // "<command> :Not enough parameters" - Returned by the server by numerous commands to indicate to the client that it didn't supply enough parameters.

#endif
