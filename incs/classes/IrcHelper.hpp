#pragma once

#include <iostream>						// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <sstream>						// gestion flux -> std::ostringstream
#include <ctime> 						// gestion temps -> std::time_t, std::tm
#include <map>							// container map
#include <vector>						// container vector
#include <cstdlib> 						// std::strtol
#include <climits> 						// std::numeric_limits
#include <cerrno>						// codes erreur -> errno

// === NAMESPACES ===
#include "../config/irc_config.hpp"
#include "../config/server_messages.hpp"
#include "../config/commands.hpp"
#include "../config/colors.hpp"

// === CLASSES ===
#include "Utils.hpp"
#include "MessageHandler.hpp"
#include "Client.hpp"
#include "Channel.hpp"

// =========================================================================================

class Client;
class Channel;
class IrcHelper {

	private:
		IrcHelper();
		IrcHelper(const IrcHelper& src);
		IrcHelper& operator=(IrcHelper& src);
		~IrcHelper();

	public:

		// === SERVER CONNECT HELPER ===
		static int validatePort(const std::string& port);

		// === AUTHENTICATION HELPER ===
		static int getCommand(const Client& client);
		static std::string commandToSend(const Client& client);
		static bool isCommandIgnored(const std::string& cmd, bool checkAuthCmds);
		static bool isValidPassword(const std::string& password, bool isPassServer);
		static bool isValidName(const std::string& name, int type);
		static std::string formatUsername(const std::string& username);

		// === MESSAGES HELPER ===
		static std::string sanitizeIrcMessage(std::string msg, const std::string& cmd, const std::string& nickname);

		// === CHANNEL HELPER ===
		static int isRightChannel(const Client& client, const std::string& channelName, std::map<std::string, Channel*>& channels, int opt);
		static bool isValidChannelName(const std::string& channelName);
		static std::string fixChannelMask(std::string channelName);
		static bool channelExists(const std::string& channelName, std::map<std::string, Channel*>& channels);
		static bool clientExists(int clientFd);
		
		// === MODE HELPER ===
		static int isRightMode(const std::string &mode);
		static int findCharBeforeIndex(const std::string& str, char target1, char target2, size_t startPos);
		static size_t getExpectedArgCount(std::string mode);
		static std::map<char, std::string> mapModesToArgs(std::vector<std::string> args);
		static void assertNoDuplicate(std::string &str, char c, size_t i);
		static bool noChangeToMake(char modeSign, bool modeEnabled);
		static bool isValidLimit(std::string &limit);

		// === BOT HELPER ===
		static bool isBotCommandFound(const std::string& message);
		static std::string getCurrentTime();
};