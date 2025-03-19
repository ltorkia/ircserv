#pragma once

#include <iostream>				// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <sstream>				// gestion flux -> std::ostringstream
#include <fstream>				// gestion fichiers -> std::ifstream, std::ofstream
#include <vector>				// container vector

// === NAMESPACES ===
#include "../config/irc_config.hpp"
#include "../config/server_messages.hpp"

// === CLASSES ===
#include "IrcHelper.hpp"
// #include "MessageHandler.hpp"
// #include "Client.hpp"

class Bot {

	public:
	
		Bot(const std::string& nick, const std::string& user, const std::string& realName);
		~Bot();

		std::string getBotMask() const;
		std::string handleCommand(const std::string& command);
		std::string getJoke();
        std::string getAge(const std::string &input);
		std::string splitBuff(std::string buff, std::string &date);
		int parseAge(std::string age);
		void ageCalculator(std::string age, std::string Nickname,int ircsock);
		int getQuotes(std::string filename);
		std::string getQuotes(std::vector<std::string>& quotes, int size);
	
	private:

		Bot();
		Bot(const Bot& src);
		Bot& operator=(Bot& src);

		std::string _nickname;
		std::string _username;
		std::string _realName;
		std::string _hostname;
		std::string _ipAdress;
		
		std::vector<std::string> _quotes;
	};	