/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 12:12:49 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

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

Command::Command(Server& server, std::map<int, Client*>::iterator it)
	: _server(server), _it(it), _clientFd(_it->first), _client(_it->second),
	_clients(_server.getClients()), _channels(_server.getChannels())
{
	// === AUTHENTICATE COMMANDS : Command_Auth.cpp ===
	_fctMap[PASS] 			= &Command::_isRightPassword;
	_fctMap[NICK] 			= &Command::_setNicknameClient;
	_fctMap[USER] 			= &Command::_setUsernameClient;
	_fctMap[CAP] 			= &Command::_handleCapabilities;
	
	// === CHANNEL COMMANDS : Command_Channel.cpp ===
	_fctMap[INVITE] 		= &Command::_inviteChannel;
	_fctMap[JOIN] 			= &Command::_joinChannel;
	_fctMap[TOPIC] 			= &Command::_setTopic;
	_fctMap[KICK] 			= &Command::_kickChannel;
	_fctMap[PART] 			= &Command::_quitChannel;

	// === MODE COMMANDS : Command_Mode.cpp ===
	_fctMap[MODE] 			= &Command::_handleMode;

	// === MESSAGE COMMANDS : Command_Message.cpp ===
	_fctMap[PRIVMSG] 		= &Command::_sendPrivateMessage;

	// === LOG COMMANDS : Command_Log.cpp ===
	_fctMap[PING] 			= &Command::_sendPong;
	_fctMap[PONG] 			= &Command::_updateActivity;
	_fctMap[WHO] 			= &Command::_handleWho;
	_fctMap[WHOIS] 			= &Command::_handleWhois;
	_fctMap[WHOWAS] 		= &Command::_handleWhowas;
	_fctMap[AWAY] 			= &Command::_setAway;
	_fctMap[QUIT] 			= &Command::_quitServer;

	// === FILE COMMANDS : Command_File.cpp ===
	_fctMap[DCC] 			= &Command::_handleFile;
}

Command::~Command() {}

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
void Command::manageCommand(std::string input)
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
	
	std::map<std::string, void (Command::*)()>::iterator itFunction = _fctMap.find(cmd);;
	if (itFunction == _fctMap.end())
		throw std::invalid_argument(MessageBuilder::ircUnknownCommand(nickname, input));
	
	_itInput++;

	if (Utils::paramCheckNeeded(cmd) && Utils::isEmptyOrInvalid(_itInput, _vectorInput))
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(nickname, cmd));

	(this->*itFunction->second)();
}