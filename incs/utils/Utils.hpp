#pragma once

#include <iostream>						// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <sstream>						// gestion flux -> std::ostringstream
#include <fstream>						// gestion fichiers -> std::ifstream, std::ofstream
#include <vector>						// container vector
#include <cstring>						// memset()
#include <ctime> 						// gestion temps -> std::time_t, std::tm

// =========================================================================================

class Utils
{
	private:
		Utils();
		Utils(const Utils& src);
		Utils& operator=(const Utils& src);
		~Utils();

	public:

		// === TIME ===
		static void getCurrentTime(std::tm& outTime);

		// === ENV ===
		static void writeEnvFile(const std::string& serverIp, int port, const std::string& password);
		static std::string getEnvValue(const std::string& key);

		// === PARSING HELPER ===
		static bool paramCheckNeeded(const std::string& cmd);
		static bool isEmptyOrInvalid(const std::vector<std::string>::iterator& it, const std::vector<std::string>& vec);
		static bool isOnlySpace(const std::string& str);
		static bool isPrintableSentence(const std::string& str);
		static bool isNonPrintableChar(char c);
		static bool isOnlyAlphaNum(const std::string& str);
		static bool isAllDigit(const std::string& str);
		static void printVector(const std::vector<std::string>& vec);

		// === STRING MANIPULATION ===
		static std::vector<std::string> getTokens(const std::string &s, int opt);
		static std::string stockVector(std::vector<std::string>::iterator it, const std::vector<std::string>& vec);
		static std::string streamArg(const std::string& arg);
		static void toUpper(std::string& str);
		static std::string truncateStr(const std::string& str);
		static std::string emptyQuotesToEmptyString(const std::string& str);

		// === BUFFER CLEANER ===
		static std::string extractAndCleanMessage(std::string& bufferMessage, size_t pos);
};