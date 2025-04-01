/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 12:12:49 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandHandler.hpp"

// === OTHER CLASSES ===
#include "Utils.hpp"
#include "MessageBuilder.hpp"

// === NAMESPACES ===
#include "commands.hpp"
#include "server_messages.hpp"

using namespace commands;

// =========================================================================================

// === COMMAND HANDLER ===

// ========================================= PUBLIC ========================================

// === CONSTRUCTOR (INIT COMMAND HANDLERS MAP) / DESTRUCTOR ===

CommandHandler::CommandHandler(Server& server, std::map<int, Client*>::iterator it)
	: _server(server), _it(it), _clientFd(_it->first), _client(_it->second),
	_clients(_server.getClients()), _channels(_server.getChannels())
{
	// === AUTHENTICATE COMMANDS : CommandHandler_Auth.cpp ===
	_fctMap[PASS] 			= &CommandHandler::_isRightPassword;
	_fctMap[NICK] 			= &CommandHandler::_setNicknameClient;
	_fctMap[USER] 			= &CommandHandler::_setUsernameClient;
	_fctMap[CAP] 			= &CommandHandler::_handleCapabilities;
	
	// === CHANNEL COMMANDS : CommandHandler_Channel.cpp ===
	_fctMap[INVITE] 		= &CommandHandler::_inviteChannel;
	_fctMap[JOIN] 			= &CommandHandler::_joinChannel;
	_fctMap[TOPIC] 			= &CommandHandler::_setTopic;
	_fctMap[KICK] 			= &CommandHandler::_kickChannel;
	_fctMap[PART] 			= &CommandHandler::_quitChannel;

	// === MODE COMMANDS : CommandHandler_Mode.cpp ===
	_fctMap[MODE] 			= &CommandHandler::_handleMode;

	// === MESSAGE COMMANDS : CommandHandler_Message.cpp ===
	_fctMap[PRIVMSG] 		= &CommandHandler::_sendPrivateMessage;

	// === LOG COMMANDS : CommandHandler_Log.cpp ===
	_fctMap[PING] 			= &CommandHandler::_sendPong;
	_fctMap[PONG] 			= &CommandHandler::_updateActivity;
	_fctMap[WHO] 			= &CommandHandler::_handleWho;
	_fctMap[WHOIS] 			= &CommandHandler::_handleWhois;
	_fctMap[WHOWAS] 		= &CommandHandler::_handleWhowas;
	_fctMap[AWAY] 			= &CommandHandler::_setAway;
	_fctMap[QUIT] 			= &CommandHandler::_quitServer;

	// === FILE COMMANDS : CommandHandler_File.cpp ===
	_fctMap[DCC] 			= &CommandHandler::_handleFile;
}

CommandHandler::~CommandHandler() {}

// =========================================================================================

// === COMMAND MANAGER : MAIN METHOD ===

/**
 * @brief Manages the incoming command string and executes the corresponding command function.
 *
 * This function processes the command string received from the client, tokenizes it, and determines
 * the appropriate command to execute. It handles authentication checks, command validation, and
 * parameter validation before invoking the corresponding command function.
 *
 * @param input The command string received from the client.
 *
 * @throws std::invalid_argument if the command is unknown or if there are insufficient parameters.
 */
void CommandHandler::manageCommand(std::string input)
{
	if (input.empty())
		return;
		
	_vectorInput = Utils::getTokens(input, splitter::SENTENCE);	
	_itInput = _vectorInput.begin();

	std::string nickname = _client->isAuthenticated() ? _client->getNickname() : "*";
	if (Utils::isEmptyOrInvalid(_itInput, _vectorInput))
		throw std::invalid_argument(MessageBuilder::ircUnknownCommand(nickname, " "));
	
	Utils::toUpper(*_vectorInput.begin());
		
	if (_client->isAuthenticated() == false)
	{
		_authenticate();
		return ;
	}
	
	std::string cmd = *_itInput;
	
	std::map<std::string, void (CommandHandler::*)()>::iterator itFunction = _fctMap.find(cmd);;
	if (itFunction == _fctMap.end())
		throw std::invalid_argument(MessageBuilder::ircUnknownCommand(nickname, input));
	
	_itInput++;

	if (Utils::paramCheckNeeded(cmd) && Utils::isEmptyOrInvalid(_itInput, _vectorInput))
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(nickname, cmd));

	(this->*itFunction->second)();
}