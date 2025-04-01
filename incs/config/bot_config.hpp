/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot_config.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:38:24 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include "commands.hpp"

namespace bot_config
{
	// === BOT INFOS ===
	const std::string BOTNICK 						= "botty";
	const std::string BOTUSER 						= "bot";
	const std::string BOTREAL 						= "bot";

	// === BOT COMMANDS ===
	const std::string NOTICE_CMD 					= "NOTICE";
	const std::string FUNFACT_CMD					= "!funfact";
	const std::string AGE_CMD 						= "!age";
	const std::string TIME_CMD 						= "!time";

	// === AUTHENTICATION LIMITS ===
	const int REQUIRED_AUTH_SERVER_REPLY_COUNT 		= 25;

	// === FUN FACTS PATH ===
	const std::string QUOTES_PATH 					= "assets/quotes.txt";

	// === BOT MESSAGES ===
	const std::string BOT_CONNECTED 				= "🤖 Bot connected to server!";
	const std::string BOT_DISCONNECTED				= "⬅️  Bot disconnected from server";
	const std::string BOT_AUTHENTICATED 			= "✅ Bot successfully authenticated!";
	const std::string BOTNICK_CMD					= commands::NICK + " " + BOTNICK;
	const std::string BOTUSER_CMD					= commands::USER + " " + BOTUSER + " 0 * :" + BOTREAL;
	const std::string MSG_SENDING_PASSWORD			= "🔑 Sending password...";
	const std::string MSG_SENDING_NICKNAME			= "🏷️  Sending nickname...";
	const std::string MSG_SENDING_USERNAME			= "🆔 Sending username...";
	const std::string MSG_WELCOME_PROMPT			= "👋 Hello there! Type !funfact for a fun fact, !time to get the current time, or !age <YYYY-MM-DD> to know your precise age, including months and days.";
	const std::string ERR_ECHEC_CONNECTION 			= "Connection failed";
	const std::string ERR_SET_BOT_NON_BLOCKING 		= "Failed to set bot socket to non-blocking";
	const std::string ERR_AUTHENTICATION_FAILED		= "Bot authentication failed";
	const std::string ERR_AUTHENTICATION_INFO		= "Error found during authentication";
	const std::string ERR_AUTHENTICATION_TIMEOUT	= "Authentication timeout";
	const std::string ERR_WRITE_SERVER				= "Failed to send message";
	const std::string ERR_ENV_VALUE 				= "Failed to get environment value";
	const std::string ERR_UNKNOWN_BOT_CMD 			= "Unknown bot command";
	const std::string ERR_INVALID_CMD_FORMAT 		= "Invalid command format";
	const std::string INVALID_DATE_FORMAT 			= "Please enter a valid date/format: !age <YYYY-MM-DD>";
}