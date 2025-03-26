#pragma once

#include <iostream>				// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <sstream>				// gestion flux -> std::ostringstream
#include <fstream>				// gestion fichiers -> std::ifstream, std::ofstream
#include <algorithm>			// std::remove_if()
#include <vector>				// container vector
#include <map>					// container map
#include <cstring>				// memset()
#include <arpa/inet.h>			// inet_addr()
#include <sys/select.h>			// select()
#include <unistd.h>				// close()
#include <csignal>				// gestion signaux -> SIGINT, SIGTSTP

// =========================================================================================

class Bot
{
	public:

		// === SIGNAL : Bot.cpp ===
		static volatile sig_atomic_t signalReceived;
		static void signalHandler(int signal);
		void setSignal();

		// === CONSTUCTORS / DESTRUCTORS : Bot.cpp ===
		Bot(int botFd, const std::string& nick, const std::string& user, const std::string& real);
		~Bot();

		// === LISTEN ACTIVITY : Bot.cpp ===
		void run();

	
	private:

		// === BOT INFOS ===
		bool _hasSentAuthInfos, _isAuthenticated;
		int _botFd;
		std::string _botNick, _botUser, _botReal, _botMask;

		// === BUFFER ===
		std::string _buffer;

		// === CURRENT CLIENT AND CHANNEL ===
		std::string _target, _clientNickname, _channelName;

		// === CLIENT INPUT ===
		std::string _input, _command, _ageArg;
		size_t _commandPos;

		// === ARG BIRTHDATE AND CURRENT DATE ===
		int _year, _month, _day;
		int _currentYear, _currentMonth, _currentDay;
		
		// === QUOTES FOR FACTS ===
		std::vector<std::string> _quotes;

		// === ACTIVE CLIENTS ===
		std::map<std::string, bool> _activeClients;

		// ================================================================================

		// === RESET INFOS : Bot.cpp ===
		void _resetInfos();

		// === READ / SEND MESSAGES : Bot_MessageHandler.cpp ===
		void _handleMessage();
		int _readFromServer();
		void _sendMessage(const std::string &message) const;
		void _announceBotFeatures();

		// === AUTHENTICATE : Bot_Authenticate.cpp ===
		void _authenticate(const std::string& message);
		void _sendAuthInfos();

		// === SERVER COMMAND PARSER : Bot_CommandParser.cpp ===
		bool _parsePrivmsg(std::string& input);
		bool _extractSenderNick(std::string& nickname);
		bool _isRightCommand(const std::vector<std::string>& args, const std::string& command);
		bool _extractTarget(const std::vector<std::string>& args);
		std::string _extractMessage(std::vector<std::string>& args);
		
		// === BOT COMMAND PARSER : Bot_CommandParser.cpp ===
		bool _parseBotCommand(std::string& message);
		bool _isValidBotCommand(const std::string& command);
		bool _parseBirthdate();
		bool _extractDate(const std::string& dateStr);
		bool _isValidDate();

		// === SERVER COMMAND HANDLER : Bot_CommandHandler.cpp ===
		void _manageServerCommand(std::string& message);
		bool _handlePing(const std::string& input);
		bool _handleInvite(const std::string& input);
		bool _handleJoin(const std::string& input);

		// === BOT COMMAND HANDLER : Bot_CommandHandler.cpp ===
		std::string _handleBotCommand();

		// ---> FUNFACT COMMAND
		std::string _getRandomFunfact();
		std::vector<std::string> _getQuotes(std::string filename);

		// ---> AGE COMMAND
        std::string _getAge();
		std::string _ageCalculator();

		// ---> TIME COMMAND
		std::string _getTimeString();
};