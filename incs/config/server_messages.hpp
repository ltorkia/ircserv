/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_messages.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:50:47 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include "irc_config.hpp"

namespace server_messages
{
	// === SUCCESS SETTINGS ===
	const std::string SERVER_SHUT_DOWN 				= "Server successfully shut down";

	// === EXCEPTIONS FOR ERROR SETTINGS ===
	const std::string ERR_INVALID_ARGUMENTS 		= "./" + server::NAME + " <port> <password>";				// argc
	const std::string ERR_NO_NETWORK 				= "No network interface available";
	const std::string ERR_INVALID_PORT_NUMBER 		= "Invalid port number. Port must be between 1 and 65535";	// arg[1]
	const std::string ERR_INVALID_PASSWORD 			= "Incorrect password format";								// arg[2]
	const std::string ERR_SET_SIGNAL 				= "Failed to configure SIGINT signal";
	const std::string ERR_SOCKET_CREATION 			= "Failed to create server socket";
	const std::string ERR_SET_SOCKET 				= "Failed to set socket";
	const std::string ERR_SET_SERVER_NON_BLOCKING 	= "Failed to set server socket to non-blocking";
	const std::string ERR_SET_CLIENT_NON_BLOCKING 	= "Failed to set client socket to non-blocking";
	const std::string ERR_SELECT_SOCKET 			= "Failed to select socket";
	const std::string ERR_BIND_SOCKET 				= "Failed to bind server socket. Address already in use";
	const std::string ERR_LISTEN_SOCKET 			= "Failed to listen on server socket";
	const std::string ERR_ACCEPT_CLIENT 			= "Failed to accept client";
	const std::string ERR_OPEN_FILE 				= "Failed to open file";
	const std::string ERR_OPEN_ENV_FILE 			= "Failed to open environment file";
	const std::string ERR_HOME_NOT_FOUND			= "HOME environment variable not set or inaccessible";
	

	// === NOTICE MESSAGES FOR CLIENTS ===

	// --- CONNECT :
	const std::string MSG_CONNECTION_CLOSED 		= "Connection closed by server";
	const std::string MSG_BASIC_ERROR 				= "An error occurred. Please try again";

	// --- COMMANDES PROMPT
	const std::string PASS_PROMPT 					= "- Server password: PASS <password>";
	const std::string NICKNAME_PROMPT 				= "- Nickname: NICK <your_nickname> | len <= 10";
	const std::string USERNAME_PROMPT 				= "- Username: USER <username> <hostname> <servername> :<realname>";

	// --- COMMANDES REUSSIES
	const std::string SERVER_PASSWORD_FOUND 		= "Server password found";
	const std::string PROMPT_ONCE_REGISTERED 		= "You can now join a channel and start chatting!";

	
	// ===  QUIT REASONS TO SEND TO CLIENTS ===

	// --- CLIENT SIDE
	const std::string DEFAULT_REASON 				= "Bye bye everyone";
	const std::string DEFAULT_KICK_REASON 			= "You smell like Gargamel";

	// --- SERVER SIDE (normal reasons)
	const std::string CLIENT_CLOSED_CONNECTION 		= "Client left server";

	// --- SERVER SIDE (treated as errors)
	const std::string SHUTDOWN_REASON 				= "Server shutting down";
	const std::string CONNECTION_FAILED 			= "Connection failed";
	const std::string CONNECTION_TIMEOUT 			= "Connection timeout";
}