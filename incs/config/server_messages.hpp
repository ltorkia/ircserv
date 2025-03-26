#pragma once

#include <string>
#include "irc_config.hpp"
#include "commands.hpp"
#include "bot.hpp"

namespace server_messages
{
	// === SUCCESS SETTINGS ===
	const std::string SERVER_SHUT_DOWN 				= "Server successfully shut down";

	// === EXCEPTIONS FOR ERROR SETTINGS ===
	const std::string ERR_INVALID_ARGUMENTS 		= "./" + server::NAME + " <port> <password>";				// argc
	const std::string ERR_NO_NETWORK 				= "No network interface available";
	const std::string ERR_INVALID_PORT_NUMBER 		= "Invalid port number. Port must be between 1 and 65535"; 	// arg[1]
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
	const std::string ERR_ENV_VALUE 				= "Failed to get environment value";
	

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


	// === BOT ===
	const std::string BOT_CONNECTED 				= "🤖 Bot connected to server!";
	const std::string BOT_DISCONNECTED				= "⬅️  Bot disconnected from server";
	const std::string BOT_AUTHENTICATED 			= "✅ Bot successfully authenticated!";
	const std::string BOTNICK_CMD					= commands::NICK + " " + bot::NICK;
	const std::string BOTUSER_CMD					= commands::USER + " " + bot::USER + " 0 * :" + bot::REALNAME;
	const std::string MSG_SENDING_PASSWORD			= "🔑 Sending password...";
	const std::string MSG_SENDING_NICKNAME			= "🏷️  Sending nickname...";
	const std::string MSG_SENDING_USERNAME			= "🆔 Sending username...";
	const std::string MSG_WELCOME_PROMPT			= "👋 Hello there! Type !funfact for a fun fact, !time to get the current time, or !age <YYYY-MM-DD> to know your precise age, including months and days.";
	const std::string ERR_ECHEC_CONNECTION 			= "Connection failed";
	const std::string ERR_SET_BOT_NON_BLOCKING 		= "Failed to set bot socket to non-blocking";
	const std::string ERR_WRITE_SERVER				= "Failed to send message";
	const std::string ERR_INVALID_CMD_FORMAT 		= "Invalid command format";
	const std::string INVALID_DATE_FORMAT 			= "Please enter a valid date/format: !age <YYYY-MM-DD>";
}