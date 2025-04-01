/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc_config.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:38:15 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

// === SERVER INFOS / SETTINGS ===
namespace server
{
	const std::string NAME 					= "ircserv";
	const std::string LOCALHOST 			= "127.0.0.1";
	const std::string VERSION				= "1.1";
	const std::string UNKNOWN_IP 			= "unknown IP";

	const size_t BUFFER_SIZE 				= 510;

	const int PING_INTERVAL 				= 240;
	const int PONG_TIMEOUT 					= 300;
}

// === ENV INFOS ===
namespace env
{
	const std::string PATH 					= ".env";
	const std::string SERVER_IP_KEY 		= "SERVER_IP";
	const std::string SERVER_PORT_KEY 		= "SERVER_PORT";
	const std::string PASS_KEY 				= "PASSWORD";
}

// === SPLITTER MODE ===
namespace splitter
{
	enum Code 
	{
		SENTENCE  							= 1,
		WORD  								= 2,
		COMMA		 						= 3
	};
}

// === AUTHENTICATION COMMANDS TO DO ===
namespace auth_cmd
{
	enum Code
	{
		PASS_CMD  							= 0,
		NICK_CMD  							= 1,
		USER_CMD  							= 2,
		CMD_ALL_SET		   					= 3
	};
}

// === USER NAME TYPES ===
namespace name_type
{
	enum Code
	{
		NICKNAME  							= 1,
		USERNAME  							= 2,
		HOSTNAME  							= 3,
		REALNAME  							= 4
	};
}

// === CHANNEL ERRORS ===
namespace channel_error
{
	enum Code
	{
		INVALID_FORMAT  					= 1,
		NO_FOUND  							= 2,
		ALL_RIGHT  							= 3
	};
}

// === FILE COMMAND TYPES ===
namespace file
{
	const std::string SEND_CMD 				= "SEND";
	const std::string GET_CMD 				= "GET";
}

// === REASON FOR LEAVING CHANNEL ===
namespace leaving_code
{
	enum Reason
	{
		KICKED  							= 1,
		LEFT  								= 2,
		QUIT_SERV  							= 3
	};
}

// === PRINT ERRORS ===
namespace error_display
{
	enum Code
	{
		PRINT_ERROR  						= 1,
		HIDE_ERROR  						= 2
	};
}

// === INT BOOLEANS ===
namespace boolean
{
	enum Status 
	{
		FALSE  								= 0,
		TRUE  								= 1
	};
}

// === END OF LINES ===
namespace eol
{
	const std::string UNIX  				= "\n";
	const std::string IRC   				= "\r\n";
}