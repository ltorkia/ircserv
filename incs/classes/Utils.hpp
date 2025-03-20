#pragma once

#include <iostream>						// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <sstream>						// gestion flux -> std::ostringstream
#include <vector>						// container vector

// === NAMESPACES ===
#include "../config/irc_config.hpp"
#include "../config/commands.hpp"

// =========================================================================================

class Utils {

	private:
		Utils();
		Utils(const Utils& src);
		Utils& operator=(const Utils& src);
		~Utils();

	public:

		// === PARSING HELPER ===
		static bool paramCheckNeeded(const std::string& cmd);
		static bool isEmptyOrInvalid(const std::vector<std::string>::iterator& it, const std::vector<std::string>& vec);
		static bool isOnlySpace(const std::string &str);
		static bool isPrintableSentence(const std::string &str);
		static bool isOnlyAlphaNum(const std::string &str);
		static bool isAllDigit(const std::string &str);
		static void printVector(const std::vector<std::string>& vec);

		// === STRING MANIPULATION ===
		static std::vector<std::string> getTokens(const std::string &s, int opt);
		static std::string stockVector(std::vector<std::string>::iterator it, const std::vector<std::string>& vec);
		static std::string streamArg(const std::string& arg);
		static void transformingMaj(std::string &str);
		static std::string truncateStr(const std::string& str);
};