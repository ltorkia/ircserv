#include "../../incs/utils/Utils.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/commands.hpp"
#include "../../incs/config/server_messages.hpp"

using namespace commands;
using namespace server_messages;

// ========================================= PRIVATE =======================================

Utils::Utils() {}
Utils::Utils(const Utils& src) {(void) src;}
Utils & Utils::operator=(const Utils& src) {(void) src; return *this;}
Utils::~Utils() {}


// ========================================= PUBLIC ========================================

// === TIME ===

/**
 * @brief Retrieves the current local time.
 *
 * This function fills the provided std::tm structure with the current local time.
 *
 * @param[out] outTime A reference to a std::tm structure that will be filled with the current local time.
 */
void Utils::getCurrentTime(std::tm &outTime)
{
	std::time_t now = std::time(0);
	std::memset(&outTime, 0, sizeof(outTime));
	std::tm *localTime = std::localtime(&now);
	if (localTime)
		outTime = *localTime;
}


// === ENV ===

/**
 * @brief Writes server configuration details to an environment file.
 *
 * This function creates or overwrites an environment file with the provided
 * server IP address, port, and password. The file is opened in truncation mode,
 * meaning any existing content will be erased.
 *
 * @param serverIp The IP address of the server to be written to the file.
 * @param port The port number of the server to be written to the file.
 * @param password The password to be written to the file.
 *
 * @throws std::runtime_error If the file cannot be created or opened.
 */
void Utils::writeEnvFile(const std::string& serverIp, int port, const std::string& password)
{
	// Ouvre en mode écriture et écrase le contenu existant
	std::ofstream file(env::PATH.c_str(), std::ios::trunc);
	if (!file)
		throw std::runtime_error(ERR_OPEN_ENV_FILE);

	file << env::SERVER_IP_KEY << "=" << serverIp << '\n';
	file << env::SERVER_PORT_KEY << "=" << port << '\n';
	file << env::PASS_KEY << "=" << password << '\n';
	file.close();
}

/**
 * @brief Retrieves the value of an environment variable from a file.
 *
 * This function reads a file specified by `env::PATH` to find the value
 * of the environment variable corresponding to the given key. The file
 * is expected to contain key-value pairs in the format `KEY=VALUE`.
 *
 * @param key The name of the environment variable to search for.
 * @return The value of the environment variable if found, or an empty
 *         string if the key is not present in the file.
 * @throws std::runtime_error If the file specified by `env::PATH` cannot
 *         be opened.
 */
std::string Utils::getEnvValue(const std::string& key)
{
	std::ifstream file(env::PATH.c_str());
	if (!file)
		throw std::runtime_error(ERR_OPEN_ENV_FILE);

	std::string line;
	while (std::getline(file, line))
	{
		if (line.find(key) != std::string::npos)
		{
			file.close();
			return line.substr(line.find('=') + 1);
		}
	}
	file.close();
	return "";
}


// === PARSING HELPER ===

/**
 * @brief Checks if parameter check is needed for a given command.
 *
 * This function determines whether a parameter check is required for the specified command
 * (it may be checked later).
 * It returns true if the command is not one of the following: QUIT, AWAY, NICK, PRIVMSG, WHOIS, PING, or PONG.
 *
 * @param cmd The command to check.
 * @return true if parameter check is needed, false otherwise.
 */
bool Utils::paramCheckNeeded(const std::string &cmd)
{
	if (cmd != QUIT && cmd != AWAY && cmd != NICK && cmd != PRIVMSG
		&& cmd != WHOIS && cmd != PING && cmd != PONG)
		return true;
	return false;
}

/**
 * @brief Checks if the iterator points to an empty or invalid string in the vector.
 *
 * This function determines if the iterator `it` points to an element in the vector `vec` that is either:
 * - The end of the vector.
 * - A string containing only spaces.
 * - A string that is not a printable sentence.
 *
 * @param it An iterator pointing to an element in the vector `vec`.
 * @param vec A vector of strings to be checked.
 * @return true if the iterator points to an empty or invalid string, false otherwise.
 */
bool Utils::isEmptyOrInvalid(const std::vector<std::string>::iterator& it, const std::vector<std::string>& vec)
{
	if (it == vec.end() || isOnlySpace(*it) == true || isPrintableSentence(*it) == false)
		return true;
	return false;
}

/**
 * @brief Checks if the given string contains only whitespace characters.
 *
 * This function iterates through each character in the input string and 
 * checks if it is a whitespace character. If any character is not a 
 * whitespace character, the function returns false. Otherwise, it returns true.
 *
 * @param str The string to be checked.
 * @return true if the string contains only whitespace characters, false otherwise.
 */
bool Utils::isOnlySpace(const std::string &str)
{
	for (unsigned long i = 0; i < str.length(); i++)
		if (std::iswspace(str[i]) == 0)
			return (false);
	return (true);
}

/**
 * @brief Checks if a given string is a printable sentence.
 *
 * This function iterates through each character of the input string and checks
 * for the presence of non-printable characters such as tabulation, vertical tab,
 * form feed, and arrow keys (directional keys).
 *
 * @param str The input string to be checked.
 * @return true if the string is a printable sentence, false otherwise.
 */
bool Utils::isPrintableSentence(const std::string &str)
{
	for (unsigned long i = 0; i < str.length(); i++)
	{
		// Tabulation, vertical tab, form feed
		if (str[i] == '\t' || str[i] == '\v' || str[i] == '\f')
			return (false);

		// Check les touches directionnelles (flèches)
		if (str[i] == '\x1b')
			if (i + 2 < str.length())
				if (str[i + 1] == '[' && (str[i + 2] == 'A' || str[i + 2] == 'B' || str[i + 2] == 'C' || str[i + 2] == 'D'))
					return false;
	}
	return (true);
}

/**
 * @brief Checks if a character is a non-printable character.
 * 
 * This function determines whether the given character is a non-printable
 * character by using the standard library function `isprint`.
 * 
 * @param c The character to be checked.
 * @return true if the character is non-printable, false otherwise.
 */
bool Utils::isNonPrintableChar(char c)
{
	return !isprint(c);
}

/**
 * @brief Checks if the given string contains only alphabetic characters.
 *
 * This function iterates through each character in the input string and checks
 * if it is an alphabetic character. If any character is not an alphabetic character,
 * the function returns false. Otherwise, it returns true.
 *
 * @param str The string to be checked. The function does not modify the input string.
 * @return true If all characters in the string are alphabetic, false otherwise.
 */
bool Utils::isOnlyAlphaNum(const std::string &str)
{
	for (unsigned long i = 0; i < str.length(); i++)
		if (std::isalpha(str[i]) == 0)
			return (false);
	return (true);
}

/**
 * @brief Checks if a given string contains only digits.
 *
 * This function takes a string as input and checks if all characters in the string
 * are digits. If any character is not a digit, the function throws an invalid_argument
 * exception with a custom error message. If all characters are digits, the function
 * returns true.
 *
 * @param str A reference to the string to be checked.
 * @return true If all characters in the string are digits.
 */
bool Utils::isAllDigit(const std::string &str)
{
	int num;
	std::stringstream(str) >> num;
	for (unsigned long i = 0; i < str.size(); i++)
	{
		char c = str[i];
		if (!std::isdigit(c) || num == 0)
			return false;
	}
	return true;
}

/**
 * @brief Prints the contents of a vector of strings to the standard output.
 * 
 * This function takes a vector of strings as input and prints each string
 * in the vector to the standard output, one per line.
 * 
 * @param vec A constant reference to a vector of strings to be printed.
 */
void Utils::printVector(const std::vector<std::string>& vec)
{ 
	std::cout << std::endl << "Print vector = ";
	for (std::vector<std::string>::const_iterator it = vec.begin(); it != vec.end(); ++it)
		std::cout << *it << std::endl;
}


// === STRING MANIPULATION ===

/**
 * @brief Splits a given string into tokens based on the specified option.
 * 
 * @param s The input string to be tokenized.
 * @param opt The option for tokenization:
 *            - SEN: Splits the string into the first token and the rest of the string.
 *            - WORD: Splits the string into individual words.
 *            - COMMA: Splits the string by commas.
 * @return std::vector<std::string> A vector containing the resulting tokens.
 * @throws std::invalid_argument If an invalid option is provided.
 */
std::vector<std::string> Utils::getTokens(const std::string &s, int opt)
{
	std::vector<std::string> tokens;
	std::istringstream stream(s);
	std::string token;

	if (opt == splitter::SENTENCE)							// dans le cas ou on veut un token + la suite des arguments stockes dans une string
	{
		while (stream >> token)
		{
			tokens.push_back(token);
			if (std::getline(stream, token))
			{
				if (isOnlySpace(token) == true)				// verifie si la string est composee uniquement d espace, 
					return tokens;							// dans ce cas, retourne uniquement le 1er argument sans la string
				token = token.substr(1);					// supprime le premier espace de la string
				tokens.push_back(token);					// push la string en 2eme element a traiter 
			}
		}
	}
	else if (opt == splitter::WORD)							//recupere tous les arguments separements
	{
		while (stream >> token)
			tokens.push_back(token);
	}
	else if (opt == splitter::COMMA)						//dans le cas ou on doit prendre plusieurs channels ou noms de clients
	{
		unsigned long pos_coma = 0;
		unsigned long init = 0;
		while (1) 
		{
			token = s.substr(init, s.find(',', pos_coma + 1) - init);
			tokens.push_back(token);
			if (s.find(',', pos_coma + 1) == std::string::npos)
				break ;
			pos_coma = s.find(',', pos_coma + 1);
			init = pos_coma + 1;
		}
	}
	else
		throw std::invalid_argument("no opt option"); 
	return tokens;
}

/**
 * @brief Concatenates elements of a vector from a given iterator to the end of the vector into a single string.
 *
 * This function takes an iterator pointing to a position in a vector of strings and concatenates all elements
 * from that position to the end of the vector into a single string, with each element separated by a space.
 *
 * @param it An iterator pointing to the starting position in the vector from which to begin concatenation.
 * @param vec The vector of strings to concatenate.
 * @return A single string containing all elements from the iterator position to the end of the vector, separated by spaces.
 */
std::string Utils::stockVector(std::vector<std::string>::iterator it, const std::vector<std::string>& vec)
{ 
	std::string res;
	while (it != vec.end())
	{
		if (!res.empty())
			res += " ";
		res += *it;
		it++;
	}
	return res;
}

/**
 * @brief Extracts the first word from the given string argument.
 *
 * This function takes a string argument, creates an input string stream from it,
 * and extracts the first word from the stream. The extracted word is returned as a string.
 *
 * @param arg The input string from which the first word is to be extracted.
 * @return A string containing the first word extracted from the input string.
 */
std::string Utils::streamArg(const std::string& arg)
{
	std::istringstream stream(arg);
	std::string streamedArg;
	stream >> streamedArg;
	return streamedArg;
}

/**
 * @brief Transforms all printable characters in a string to uppercase.
 * 
 * This function iterates through each character in the input string and 
 * converts it to its uppercase equivalent if it is a printable character.
 * 
 * @param str The string to be transformed. The transformation is done in place.
 */
void Utils::transformingMaj(std::string &str)
{
	for (unsigned long i = 0; i < str.length(); i++)
		if (std::isprint(str[i]) != 0)
			str[i] = toupper(str[i]);
}

/**
 * @brief Truncates a given str string to a maximum length of 42 characters.
 * 
 * If the input string exceeds 42 characters, it truncates the string to 39 characters
 * and appends "..." to indicate that the string has been shortened.
 * 
 * @param str The input string to be truncated.
 * @return A truncated version of the input string if it exceeds 42 characters,
 *         otherwise the original string.
 */
std::string Utils::truncateStr(const std::string& str)
{
	return str.size() > 42 ? str.substr(0, 39) + "..." : str;
}

/**
 * @brief Checks if the given string represents a number.
 * 
 * This function iterates through each character in the string and checks if it is a digit.
 * If any character is not a digit, the function returns false.
 * 
 * @param str The string to be checked.
 * @return true if the string represents a number, false otherwise.
 */
bool Utils::isNumber(const std::string& str)
{
	for (size_t i = 0; i < str.size(); ++i)
		if (!std::isdigit(str[i])) return false;
	return true;
}


// === BUFFER CLEANER ===

/**
 * @brief Extracts and cleans a message from the buffer.
 *
 * This function extracts a message from the provided buffer up to the specified position (excluding the newline character).
 * If the extracted message ends with a carriage return character ('\r'), it is removed.
 * The processed part of the buffer is then erased.
 *
 * @param bufferMessage The buffer containing the message to be extracted and cleaned.
 * @param pos The position up to which the message should be extracted.
 * @return A cleaned message string.
 */
std::string Utils::extractAndCleanMessage(std::string& bufferMessage, size_t pos)
{
	// On extrait le message jusqu'au \n (non inclus)
	std::string message = bufferMessage.substr(0, pos);

	// On enlève le \r s'il y en a un (cas irssi)
	if (!message.empty() && message[message.size() - 1] == '\r')
		message.erase(message.size() - 1);

	// On supprime la commande traitée du buffer
	bufferMessage.erase(0, pos + 1);

	return message;
}