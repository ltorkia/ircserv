/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot_Register.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:25:01 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"

// === OTHER CLASSES ===
#include "Utils.hpp"
#include "MessageBuilder.hpp"

// === NAMESPACES ===
#include "bot_config.hpp"
#include "irc_config.hpp"
#include "server_messages.hpp"
#include "commands.hpp"
#include "colors.hpp"

using namespace bot_config;
using namespace server_messages;
using namespace commands;
using namespace colors;

// =========================================================================================

// === REGISTER ===

// ========================================= PRIVATE =======================================

/**
 * @brief Sends authentication information to the server.
 * 
 * This function performs the following steps sequentially:
 * 1. Retrieves the password from the environment variable defined by `env::PASS_KEY`.
 *    If the password is not found, it throws an `std::invalid_argument` exception with
 *    the error message `ERR_ENV_VALUE`.
 * 2. Constructs and sends the PASS command using the retrieved password.
 *    Displays a message indicating the password is being sent and waits for 1 second.
 * 3. Sends the NICKNAME command using the predefined `BOTNICK_CMD`.
 *    Displays a message indicating the nickname is being sent and waits for 1 second.
 * 4. Sends the USERNAME command using the predefined `BOTUSER_CMD`.
 *    Displays a message indicating the username is being sent and waits for 1 second.
 * 
 * @throws std::invalid_argument If the password environment variable is empty.
 */
void Bot::_sendAuthInfos() const
{
	// SEND PASS
	std::string pass = Utils::getEnvValue(env::PASS_KEY);
	if (pass.empty())
		throw std::invalid_argument(ERR_ENV_VALUE);

	const std::string BOTPASS_CMD = commands::PASS + " " + pass;

	_sendMessage(BOTPASS_CMD);
	std::cout << MSG_SENDING_PASSWORD << std::endl << std::endl;
	sleep(1);

	// SEND NICKNAME
	_sendMessage(BOTNICK_CMD);
	std::cout << MSG_SENDING_NICKNAME << std::endl << std::endl;
	sleep(1);

	// SEND USERNAME
	_sendMessage(BOTUSER_CMD);
	std::cout << MSG_SENDING_USERNAME << std::endl << std::endl;
	sleep(1);
}

/**
 * @brief Appends a server response message to the authentication buffer and increments the server message count.
 * 
 * @param message The server response message to be appended to the authentication buffer.
 */
void Bot::_saveServerResponses(const std::string& message)
{
	_authBuffer.append(message);
	_serverMsgCount++;
}

/**
 * @brief Authenticates the bot by verifying server messages and welcome message.
 *
 * This function performs the following checks:
 * 1. If the number of server messages received (_serverMsgCount) is less than
 *    the required count (REQUIRED_AUTH_SERVER_REPLY_COUNT), it checks for errors
 *    using the _errorFound() method. If an error is found, an exception is thrown
 *    with the message ERR_AUTHENTICATION_INFO. Otherwise, the function returns
 *    without proceeding further, allowing the bot to wait for more messages.
 * 2. If the number of server messages exceeds the required count, an exception
 *    is thrown with the message ERR_AUTHENTICATION_TIMEOUT.
 * 3. If the welcome message is not found (checked using _foundWelcomeMessage()),
 *    an exception is thrown with the message ERR_AUTHENTICATION_FAILED.
 *
 * Upon successful authentication:
 * - The authentication buffer (_authBuffer) is cleared.
 * - The bot's authentication status (_isAuthenticated) is set to true.
 * - A success message (BOT_AUTHENTICATED) is printed to the console.
 *
 * @throws std::invalid_argument If any of the authentication checks fail.
 */
void Bot::_authenticate()
{
	if (_serverMsgCount < REQUIRED_AUTH_SERVER_REPLY_COUNT)
	{
		if (_errorFound())
			throw std::invalid_argument(ERR_AUTHENTICATION_INFO);
		return;
	}
	if (_serverMsgCount > REQUIRED_AUTH_SERVER_REPLY_COUNT)
		throw std::invalid_argument(ERR_AUTHENTICATION_TIMEOUT);
	if (!_foundWelcomeMessage())
		throw std::invalid_argument(ERR_AUTHENTICATION_FAILED);

	_authBuffer.clear();
	_isAuthenticated = true;
	std::cout << BOT_AUTHENTICATED << std::endl << std::endl;
}

/**
 * @brief Checks if any error message is present in the authentication buffer.
 *
 * This function searches the authentication buffer (_authBuffer) for various
 * predefined error messages related to the bot's registration process. These
 * error messages include:
 * - Missing parameters for PASS, NICK, or USER commands.
 * - Already registered status.
 * - Incorrect password.
 * - Missing or erroneous nickname.
 * - Nickname already taken.
 *
 * @return true If any of the error messages are found in the authentication buffer.
 * @return false If none of the error messages are found in the authentication buffer.
 */
bool Bot::_errorFound() const
{
	std::string errorPassParam = MessageBuilder::ircNeedMoreParams(BOTNICK, PASS);
	std::string errorNickParam = MessageBuilder::ircNeedMoreParams(BOTNICK, NICK);
	std::string errorUserParam = MessageBuilder::ircNeedMoreParams(BOTNICK, USER);
	std::string errorLoggedIn = MessageBuilder::ircAlreadyRegistered(BOTNICK);
	std::string errorIncorrectPass = MessageBuilder::ircPasswordIncorrect();
	std::string errorNoNick = MessageBuilder::ircNoNicknameGiven(BOTNICK);
	std::string errorErroneusNick = MessageBuilder::ircErroneusNickname("*", BOTNICK);
	std::string errorNickTaken = MessageBuilder::ircNicknameTaken("*", BOTNICK);

	size_t errorPassParamPos = _authBuffer.find(errorPassParam);
	size_t errorNickParamPos = _authBuffer.find(errorNickParam);
	size_t errorUserParamPos = _authBuffer.find(errorUserParam);
	size_t errorLoggedInPos = _authBuffer.find(errorLoggedIn);
	size_t errorIncorrectPassPos = _authBuffer.find(errorIncorrectPass);
	size_t errorNoNickPos = _authBuffer.find(errorNoNick);
	size_t errorErroneusNickPos = _authBuffer.find(errorErroneusNick);
	size_t errorNickTakenPos = _authBuffer.find(errorNickTaken);

	if (errorPassParamPos == std::string::npos && errorNickParamPos == std::string::npos
		&& errorUserParamPos == std::string::npos && errorLoggedInPos == std::string::npos
		&& errorIncorrectPassPos == std::string::npos && errorNoNickPos == std::string::npos
		&& errorErroneusNickPos == std::string::npos && errorNickTakenPos == std::string::npos)
		return false;
	return true;
}

/**
 * @brief Checks if the welcome message for the bot is present in the authentication buffer.
 *
 * This function constructs the expected IRC welcome message for the bot using its nickname
 * and mask, and then searches for this message within the authentication buffer.
 *
 * @return true if the welcome message is found in the authentication buffer, false otherwise.
 */
bool Bot::_foundWelcomeMessage() const
{
	std::string welcomeMsg = MessageBuilder::ircWelcomeMessage(_botNick, _botMask);
	size_t pos = _authBuffer.find(welcomeMsg);
	return (pos != std::string::npos);
}