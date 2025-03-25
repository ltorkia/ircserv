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
		static volatile sig_atomic_t signalReceived;
		static void signalHandler(int signal);

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
		bool _handleSpecialCommands(const std::string& input);
		bool _handleInvite(const std::string& input);
		bool _noBotCommandFound(const std::string& input);

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
		bool _parseBirthdate();
		bool _extractDate(const std::string& dateStr);
		bool _isValidDate();
		std::string _ageCalculator();
};