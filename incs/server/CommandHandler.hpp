#pragma once

// === SERVER LIBRARIES ===
#include "../config/server_libs.hpp"

// === HIGH CLASS DEPENDENCY ===
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

// =========================================================================================

class File;
class CommandHandler
{
	public : 
		// =================================================================================
		// === COMMAND HANDLER === CommandHandler.cpp ===

		// === CONSTRUCTOR (INIT COMMAND HANDLERS MAP) / DESTRUCTOR ===
		CommandHandler(Server& server, std::map<int, Client*>::iterator it);
		~CommandHandler();

		// =================================================================================
		
		// === MAIN PUBLIC METHOD ===

		// =================================================================================

		// === COMMAND MANAGER ===
		void manageCommand(std::string input);

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
		std::map<std::string, void (CommandHandler::*)()> _fctMap;

		// === CURRENT INPUT TO VECTOR + ITERATOR ===
		std::vector<std::string> _vectorInput;
		std::vector<std::string>::iterator _itInput;
		
		// === MODE TOOLS ===
		char _modeSign;
		std::string	_mode;

		// === FILES TO SEND ===
		std::map<std::string, File>	_files;


		// =================================================================================
		
		// === PRIVATE METHODS ===

		// =================================================================================

		// =================================================================================
		// === AUTHENTICATE COMMANDS : CommandHandler_Register.cpp ===

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
		// === CHANNEL COMMANDS : CommandHandler_Channel.cpp ===

		void _inviteChannel();
		void _joinChannel();
		void _setTopic();
		void _kickChannel();
		void _quitChannel();

		// =================================================================================
		// === MODE : CommandHandler_Mode.cpp ===

		void _changeMode();
		void _applyChannelModes(std::string &mode, std::string &arg1, std::map<char, std::string> &arg2);
		void _validateModeArguments(std::string &mode, const Channel *channel, unsigned int nArgs);
		bool _validateModeCommand(std::string &channel, std::string &mode, unsigned int nArgs);
		void _setInviteOnly(Channel *channel, char modeSign);
		void _setTopicRestriction(Channel *channel, char modeSign);
		void _setPasswordMode(std::string args, Channel *channel, char modeSign, Client *client);
		void _setOperatorPrivilege(Channel *channel, char modeSign, Client *newOp);
		bool _setChannelLimit(Channel *channel, char modeSign, std::string args);

		// =================================================================================
		// === MESSAGE COMMANDS : CommandHandler_Message.cpp ===

		void _sendPrivateMessage();
		void _sendToChannel(std::vector<std::string>& targets, std::string& message);
		void _sendToClient(std::vector<std::string>& targets, std::string& message);

		// =================================================================================
		// === LOG COMMANDS : CommandHandler_Log.cpp ===

		void _sendPong();
		void _updateActivity();
		void _handleWhois();
		void _handleWhowas();
		void _handleWho();
		void _setAway();
		void _quitServer();

		// =================================================================================
		// === FILE COMMANDS : CommandHandler_File.cpp ===

		void _handleFile();
		void _sendFile(std::vector<std::string> args);
		void _getFile(std::vector<std::string> args);
		std::string _getFilename(const std::string& path) const;
		size_t _getArgsSize() const;
};
