#pragma once

#include <iostream>				// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <sstream>				// gestion flux -> std::ostringstream
#include <fstream>				// gestion fichiers -> std::ifstream, std::ofstream
#include <vector>				// container vector
#include <map>					// container map
#include <cstring>				// memset()
#include <arpa/inet.h>			// inet_addr()
#include <unistd.h>				// close()
#include <csignal>				// gestion signaux -> SIGINT, SIGTSTP

class Bot
{
	public:

		// === SIGNAL : Bot.cpp ===
		static volatile sig_atomic_t signalReceived;							// Indique si un signal a été reçu
		static void signalHandler(int signal);									// Gestionnaire de signaux

		// === CONSTUCTORS / DESTRUCTORS : Bot.cpp ===
		Bot(int botFd, const std::string& nick, const std::string& user, const std::string& real);
		~Bot();

		// === LISTEN ACTIVITY : Bot.cpp ===
		void run();
	
	private:

		// === BOT INFOS ===
		bool _hasSentAuthInfos;
		bool _isAuthenticated;
		int _botFd;
		std::string _botNick;
		std::string _botUser;
		std::string _botReal;
		std::string _botMask;

		// === CURRENT CLIENT AND CHANNEL ===
		std::string _target;
		std::string _clientNickname;
		std::string _channelName;

		// === CLIENT INPUT ===
		std::string _input;
		std::string _command;
		size_t _commandPos;
		std::string _ageArg;

		// === BUFFER ===
		std::string _buffer;
		
		// === QUOTES FOR JOKES ===
		std::vector<std::string> _quotes;

		// ================================================================================

		// === READ / SEND MESSAGES : Bot_MessageProcessor.cpp ===
		void _handleMessage();
		int _readFromServer();
		void _sendMessage(const std::string &message) const;

		// === AUTHENTICATE : Bot_Authenticate.cpp ===
		void _authenticate(const std::string& message);
		void _sendAuthInfos();
		
		// === COMMAND HANDLER : Bot_CommandHandler.cpp ===
		void _manageCommand(std::string& message);
		bool _noBotCommandFound(const std::string& input);
		bool _handleSpecialCommands(const std::string& input);
		bool _handleInvite(const std::string& input);

		// === PRIVMSG PARSER : Bot_PrivmsgParser.cpp ===
		bool _parsePrivmsg(std::string& input);
		bool _extractSenderNick(std::string& nickname);
		bool _isValidPrivmsg(const std::vector<std::string>& args);
		bool _extractTarget(const std::vector<std::string>& args);
		std::string _extractMessage(std::vector<std::string>& args);
		bool _parseBotCommand(std::string& message);

		// === PRIVMSG HANDLER : Bot_PrivmsgHandler.cpp ===
		std::string _handlePrivmsgCommand();

		// === --- JOKE COMMAND : Bot_PrivmsgHandler.cpp ===
		std::string _getRandomJoke();
		std::vector<std::string> _getQuotes(std::string filename);

		// === --- AGE COMMAND : Bot_PrivmsgHandler.cpp ===
        std::string _getAge();
		bool _parseAge();
		std::string _ageCalculator();
};