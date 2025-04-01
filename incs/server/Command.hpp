/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 12:28:40 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// === SERVER LIBRARIES ===
#include "server_libs.hpp"

// === HIGH CLASS DEPENDENCY ===
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

// =========================================================================================

class Command
{
	public : 
		// =================================================================================
		// === COMMAND HANDLER === Command.cpp ===

		// === CONSTRUCTOR (INIT COMMAND HANDLERS MAP) / DESTRUCTOR ===
		Command(Server& server, std::map<int, Client*>::iterator it);
		~Command();

		// =================================================================================
		
		// === MAIN PUBLIC METHOD ===

		// =================================================================================

		// === COMMAND MANAGER ===
		void manageCommand(std::string& input);

	private : 
		// =================================================================================
		
		// === VARIABLES ===

		// =================================================================================

		// === SERVER INSTANCE ===
		Server& _server;

		// === CURRENT CLIENT INFOS ===
		std::map<int, Client*>::iterator _it;
		int _clientFd;
		Client* _client;

		// === REFERENCE TO ALL CLIENTS + ALL CHANNELS ===
		std::map<int, Client*>& _clients;
		std::map<std::string, Channel*>& _channels;

		// === MAP : COMMANDS -> HANDLERS ===
		std::map<std::string, void (Command::*)()> _fctMap;

		// === CURRENT INPUT TO VECTOR + ITERATOR ===
		std::vector<std::string> _vectorInput;
		std::vector<std::string>::iterator _itInput;
		
		// === MODE TOOLS ===
		char _modeSign;
		std::string	_mode;
		std::map<char, std::string> _modeArgs;


		// =================================================================================
		
		// === PRIVATE METHODS ===

		// =================================================================================

		// =================================================================================
		// === AUTHENTICATE COMMANDS : Command_Register.cpp ===

		void _authenticate();
		void _preRegister(const std::string& cmd, int toDo);
		void _isRightPassword();
		void _setNicknameClient();
		void _setUsernameClient();
		void _usernameSettings(const std::vector<std::string>::iterator& itArg);
		void _hostnameSettings(std::vector<std::string>::iterator& itArg);
		void _realNameSettings(std::vector<std::string>::iterator& itArg, const std::vector<std::string>& args);
		void _handleCapabilities();

		// =================================================================================
		// === CHANNEL COMMANDS : Command_Channel.cpp ===

		void _inviteChannel();
		void _joinChannel();
		void _setTopic();
		void _kickChannel();
		void _quitChannel();

		// =================================================================================
		// === MODE : Command_Mode.cpp ===

		// === MODE PARSER ===
		void _handleMode();
		bool _validateModeCommand(const std::string& channelName, unsigned int nArgs);
		void _validateModeArguments(const Channel *channel, unsigned int nArgs);
		void _applyChangeMode(const std::string& target);
		void _setOperatorPrivilegeWrapper(Channel *channel);
		void _setChannelLimitWrapper(Channel *channel);
		
		// === MODE HANDLER ===
		void _setInviteOnly(Channel *channel);
		void _setTopicRestriction(Channel *channel);
		void _setPasswordMode(Channel *channel);
		void _setOperatorPrivilege(Channel *channel, Client *newOp);
		bool _setChannelLimit(Channel *channel);

		// =================================================================================
		// === MESSAGE COMMANDS : Command_Message.cpp ===

		void _sendPrivateMessage();
		void _sendToChannel(std::vector<std::string>& targets, std::string& message);
		void _sendToClient(std::vector<std::string>& targets, std::string& message);

		// =================================================================================
		// === LOG COMMANDS : Command_Log.cpp ===

		void _sendPong();
		void _updateActivity();
		void _handleWhois();
		void _handleWhowas();
		void _handleWho();
		void _setAway();
		void _quitServer();

		// =================================================================================
		// === FILE COMMANDS : Command_File.cpp ===

		void _handleFile();
		void _sendFile(std::vector<std::string> args);
		void _getFile(std::vector<std::string> args);
		std::string _getFilename(const std::string& path) const;
};
