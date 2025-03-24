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

		// === SIGNAL ===
		static volatile sig_atomic_t signalReceived;							// Indique si un signal a été reçu
		static void signalHandler(int signal);									// Gestionnaire de signaux
		sig_atomic_t isSignalReceived() const; 									// Vérifie si un signal a été reçu	
	
		// === CONSTUCTORS / DESTRUCTORS ===
		Bot(int botFd, const std::string& nick, const std::string& user, const std::string& real);
		~Bot();

		// === LISTEN ACTIVITY ===
		void listenActivity();
	
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

		// === COMMAND HANDLER ===
		void _readInput();
		void _authenticate();
		std::vector<std::string>::iterator _getItCommand(std::vector<std::string>& args);
		bool _parsingFailed(std::string& input);
		bool _processSpecialMessages(const std::string& input);
		bool _noBotCommandFound(const std::string& input);
		std::string _handleCommand();

		// === JOKE COMMAND ===
		std::string _getJoke();
		std::vector<std::string> _getQuotes(std::string filename);
		std::string _getRandomQuote();

		// === AGE COMMAND ===
        std::string _getAge();
		bool _parseAge();
		std::string _ageCalculator();

		// === SEND MESSAGE ===
		void _sendMessage(const std::string &message) const;
};