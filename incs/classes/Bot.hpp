#pragma once

#include <iostream>				// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <sstream>				// gestion flux -> std::ostringstream
#include <fstream>				// gestion fichiers -> std::ifstream, std::ofstream
#include <vector>				// container vector
#include <map>					// container map

// === NAMESPACES ===
#include "../config/irc_config.hpp"
#include "../config/server_messages.hpp"
#include "../config/commands.hpp"

// === CLASSES ===
#include "IrcHelper.hpp"
#include "MessageHandler.hpp"
#include "Server.hpp"
#include "Client.hpp"

class Server;
class Client;
class Bot : public Client {

	public:
	
		Bot(int botFd, const std::string& nick, const std::string& user, const std::string& real, Server& server);
		~Bot();
	
	private:

		// === SERVER / CLIENTS / CHANNELS ===
		Server& _server;
		std::map<int, Client*>& _clients;
		std::map<std::string, Channel*>& _channels;

		// === CURRENT CLIENT ===
		Client* _client;
		int _clientFd;

		// === CURRENT CHANNEL ===
		Channel* _channel;

		// === CLIENT INPUT ===
		std::string _input;
		std::string _command;
		std::string _ageArg;
		
		// === QUOTES FOR JOKES ===
		std::vector<std::string> _quotes;

		// ================================================================================

		// === INIT / LISTEN ===
		void _initBot();
		void _listenActivity();

		// === COMMAND HANDLER ===
		void _readInput();
		void _parseInput(std::string& input);
		std::string _handleCommand(std::string& input);

		// === JOKE COMMAND ===
		std::string _getJoke();
		std::vector<std::string> _getQuotes(std::string filename);
		std::string _getRandomQuote();

		// === AGE COMMAND ===
        std::string _getAge();
		bool _parseAge();
		std::string _ageCalculator();
};	