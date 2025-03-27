#include "../../incs/utils/IrcHelper.hpp"

// === OTHER CLASSES ===
#include "../../incs/server/Client.hpp"
#include "../../incs/server/Channel.hpp"
#include "../../incs/utils/Utils.hpp"
#include "../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/server_messages.hpp"
#include "../../incs/config/commands.hpp"
#include "../../incs/config/colors.hpp"

using namespace server_messages;
using namespace name_type;
using namespace auth_cmd;
using namespace commands;
using namespace channel_error;
using namespace error_display;
using namespace colors;

// ========================================= PRIVATE =======================================

IrcHelper::IrcHelper() {}
IrcHelper::IrcHelper(const IrcHelper& src) {(void) src;}
IrcHelper & IrcHelper::operator=(const IrcHelper& src) {(void) src; return *this;}
IrcHelper::~IrcHelper() {}


// ========================================= PUBLIC ========================================

// === SERVER CONNECT HELPER ===

/**
 * @brief Validates and converts a port number from a string to an integer.
 *
 * This function takes a string representation of a port number, converts it to a long integer,
 * and checks if it falls within the valid range for port numbers (1 to 65535). If the conversion
 * fails or the port number is out of range, it throws an std::invalid_argument exception.
 *
 * @param port The string representation of the port number to validate.
 * @return The validated port number as an integer.
 * @throws std::invalid_argument if the port number is invalid or out of range.
 */
int IrcHelper::validatePort(const std::string& port)
{
	char* endptr = NULL;
	errno = 0;
	long portNumber = std::strtol(port.c_str(), &endptr, 10);

	// Vérifier si la conversion a échoué ou si la valeur est hors limites
	if (errno == ERANGE || portNumber < 1 || portNumber > 65535 || *endptr != '\0')
		throw std::invalid_argument(ERR_INVALID_PORT_NUMBER);

	return static_cast<int>(portNumber);
}

// === AUTHENTICATION HELPER ===

/**
 * @brief Determines the next action to be taken based on the client's state.
 *
 * This function checks the state of the provided client and returns an integer
 * indicating the next action to be taken:
 * - 0: The server password is not valid.
 * - 1: The client's nickname does not exist.
 * - 2: The client's username does not exist.
 * - 3: All required information is valid and present.
 *
 * @param client The client whose state is being checked.
 * @return An integer indicating the next action to be taken.
 */
int	IrcHelper::getCommand(const Client& client)
{
	int to_do = CMD_ALL_SET;				//renvoie 3 si tout est fait
	
	if (client.gotValidServPassword() == false)
		to_do = PASS_CMD;					//renvoie 0 si le password n'est pas valide
	else if (client.getNickname().empty())
		to_do = NICK_CMD;					//renvoie 1 si le nickname n'existe pas
	else if (client.getUsername().empty())
		to_do = USER_CMD;					//renvoie 2 si l'user n'existe pas 
	
	return to_do;
}

/**
 * @brief Generates a command prompt string based on the client's state.
 *
 * This function checks the state of the provided client and generates a 
 * command prompt string that indicates which information is missing or 
 * required from the client. The function performs the following checks:
 * 
 * 1. If the client has not provided a valid server password, it adds 
 *    a prompt for the password.
 * 2. If the client's nickname is empty, it adds a prompt for the nickname 
 *    to the command string.
 * 3. If the client's username is empty, it adds a prompt for the username 
 *    to the command string.
 * 
 * @param client The client object whose state is being checked.
 * @return A string containing the command prompts for missing information.
 */
std::string IrcHelper::commandToSend(const Client& client)
{
	std::string command_to_send;

	if (client.gotValidServPassword() == false)
		command_to_send = PASS_PROMPT;
	if (client.getNickname().empty())
	{
		if (!(command_to_send.empty()))
			command_to_send += ",";
		command_to_send += NICKNAME_PROMPT;
	}
	if (client.getUsername().empty())
	{
		if (!(command_to_send.empty()))
			command_to_send += ",";
		command_to_send += USERNAME_PROMPT;
	}
	return command_to_send;
}

/**
 * @brief Checks if a given IRC command should be ignored.
 *
 * This function determines whether a given IRC command should be ignored based on
 * the command itself and an optional check for authentication commands.
 *
 * @param cmd The IRC command to check.
 * @param checkAuthCmds A boolean flag indicating whether to check for authentication commands.
 *                      If true, the function will consider PASS, NICK, and USER commands as not ignored.
 * @return true if the command should be ignored, false otherwise.
 */
bool IrcHelper::isCommandIgnored(const std::string& cmd, bool checkAuthCmds)
{
	// Si la commande est CAP, PONG ou QUIT, on ne l'ignore pas
	if (cmd == CAP || cmd == PONG || cmd == QUIT)
		return false;

	// Si checkAuthCmds est activé, et que la commande est PASS, NICK, ou USER, on ne l'ignore pas
	if (checkAuthCmds && (cmd == PASS || cmd == NICK || cmd == USER))
		return false;

	// Toutes les autres commandes doivent être ignorées
	return true;
}

/**
 * @brief Validates the given password based on specified criteria.
 *
 * This function checks if the provided password meets the following conditions:
 * - It is not empty.
 * - Its length is between 3 and 20 characters.
 * - If `passServer` is false, the password can only contain alphanumeric characters and underscores ('_').
 * - If `passServer` is true, the password can contain printable characters.
 *
 * @param password The password string to validate.
 * @param passServer A boolean flag indicating the type of validation:
 *                   - false: Validate for client-side password rules.
 *                   - true: Validate for server-side password rules.
 * @return true if the password is valid according to the specified criteria, false otherwise.
 */
bool IrcHelper::isValidPassword(const std::string& password, bool isPassServer)
{
	if (password.empty())
		return false;
	if (password.length() < 3 || password.length() > 20)
		return false;
	for (size_t i = 0; i < password.size(); i++)
	{
		if (isPassServer == false && !std::isalnum(password[i]) && password[i] != '_')
			return false;
		if (isPassServer == true && !std::isprint(password[i]))
			return false;
	}
	return true;
}

/**
 * @brief Checks if a given name is valid based on its type.
 * 
 * @param name The name to be validated.
 * @param type The type of the name (NICKNAME, HOSTNAME, REALNAME, USERNAME).
 * @return true if the name is valid, false otherwise.
 * 
 * The function performs the following checks:
 * - Ensures the name is not empty.
 * - Checks the length of the name based on its type:
 *   - NICKNAME: Maximum length of 10 characters.
 *   - HOSTNAME and REALNAME: Maximum length of 42 characters.
 * - For NICKNAME:
 *   - Ensures the first character is not '#', ':', '&', a space, or a digit.
 * - For all types except HOSTNAME:
 *   - Ensures there is at least one alphabetic character.
 * - Validates each character in the name:
 *   - Allows alphanumeric characters for all types.
 *   - Allows spaces for REALNAME.
 *   - Allows special characters [], {}, \, |, _, and - for NICKNAME and USERNAME.
 *   - Allows dots for HOSTNAME.
 * - Disallows any other characters.
 * - Ensures there is at least one alphabetic character for all types except HOSTNAME.
 */
bool IrcHelper::isValidName(const std::string& name, int type)
{

	// Vérifier que le name n'est pas vide
	if (name.empty() || name[0] == '\0')
		return false;

	// Vérifier la longueur du nickname, hostname et realname
	if ((type == NICKNAME && name.length() > 10)
		|| ((type == HOSTNAME || type == REALNAME) && name.length() > 42))
		return false;

	// Pour le nickname, vérifier que le premier caractère n'est pas un '#', ':', '&', un espace
	// et ne commence pas par un chiffre
	if (type == NICKNAME
			&& (name[0] == '#' || name[0] == ':' || name[0] == '&' || isspace(name[0])
				|| (name[0] >= '0' && name[0] <= '9')))
		return false;

	// Dans tous les cas sauf pour le hostname,
	// vérifier qu'il y a au moins un caractère de l'alphabet
	bool hasAlpha = false;

	// Vérifier que chaque caractère est valide
	for (size_t i = 0; i < name.length(); ++i) {
		char c = name[i];

		// Set booleen a true si alpha trouve
		if (isalpha(c))
			hasAlpha = true;

		// Autoriser les lettres et chiffres pour nickname, username et realname
		if (isalnum(c)) 
			continue;

		// Pour le realname uniquement, autoriser les espaces
		if (type == REALNAME && isspace(c)) 
			continue;

		// Pour le nickname et le username, autoriser [], {}, \, |,_ et -
		if ((type == USERNAME || type == NICKNAME)
			&& (c == '[' || c == ']' || c == '{' || c == '}' || c == '\\' || c == '|' || c == '_' || c == '-')) 
			continue;

		// Pour le hostname uniquement, autoriser les points
		if (type == HOSTNAME && c == '.') 
			continue;

		// Interdire les autres caractères
		return false;
	}

	// Return false si pas de caractere alpha, sauf si on check le hostname
	if (type != HOSTNAME && !hasAlpha)
		return false;

	return true;
}

/**
 * @brief Formats a username by prepending a tilde (~) and truncating it to a maximum length of 10 characters.
 * 
 * @param username The original username to be formatted.
 * @return A formatted username string with a tilde prepended and truncated to a maximum of 10 characters.
 */
std::string IrcHelper::formatUsername(const std::string& username)
{
	std::string res = username;
	res = "~" + res;
	if (res.length() > 10)
		res = res.substr(0, 10);
	return res;
}


// === MESSAGES HELPER ===

/**
 * @brief Sanitizes an IRC message by removing the leading character if necessary.
 *
 * This function checks if the given IRC message needs to be sanitized based on the command type.
 * If the command is PART, KICK, PRIVMSG, TOPIC, AWAY or QUIT, and the message does not start with a colon (:),
 * or if the command is QUIT_SERVER and the message does not start with a colon or is not a printable sentence,
 * it throws an invalid_argument exception.
 *
 * @param msg The IRC message to be sanitized.
 * @param cmd The IRC command associated with the message.
 * @param nickname The nickname of the user sending the message.
 * @return The sanitized IRC message with the leading character removed.
 * @throws std::invalid_argument if the message does not meet the required conditions for the given command.
 */
std::string IrcHelper::sanitizeIrcMessage(std::string msg, const std::string& cmd, const std::string& nickname)
{
	// PART, KICK PRIVMSG, TOPIC, QUIT
	if (((cmd == PART || cmd == KICK || cmd == PRIVMSG || cmd == TOPIC || cmd == AWAY) && msg[0] != ':')
		|| (cmd == QUIT && (msg[0] != ':' || Utils::isPrintableSentence(msg) == false)))
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(nickname, cmd));

	return msg.erase(0, 1);
}


// === CHANNEL HELPER ===

/**
 * @brief Checks if the given channel name is valid and exists in the provided channels map.
 *
 * This function performs two checks:
 * 1. Validates the format of the channel name.
 * 2. Checks if the channel exists in the provided channels map.
 *
 * If the format of the channel name is invalid, and the optional parameter `opt` is set to "PRINT_ERROR",
 * it sends an error message to the client indicating the bad channel name.
 *
 * If the channel does not exist, and the optional parameter `opt` is set to "PRINT_ERROR",
 * it sends an error message to the client indicating that the channel does not exist.
 *
 * @param client The client requesting the channel check.
 * @param channelName The name of the channel to check.
 * @param channels A map of existing channels.
 * @param opt Optional parameter to specify if error messages should be sent to the client ("PRINT_ERROR / HIDE_ERROR").
 * @return An integer indicating the result of the check:
 *         - INVALID_FORMAT if the channel name format is invalid.
 *         - NO_FOUND if the channel does not exist.
 *         - ALL_RIGHT if the channel name is valid and the channel exists.
 */
int IrcHelper::isRightChannel(const Client& client, const std::string& channelName, std::map<std::string, Channel*>& channels, int opt)
{
	// Check le format du nom du channel
	if (isValidChannelName(channelName) == false)
	{
		if (opt == PRINT_ERROR)
			client.sendMessage(MessageBuilder::ircBadChannelName(client.getNickname(), channelName), NULL);
		return INVALID_FORMAT;														
	}
	// Check si le channel existe
	if (channelExists(channelName, channels) == false) 
	{
		if (opt == PRINT_ERROR)
			client.sendMessage(MessageBuilder::ircNoSuchChannel(client.getNickname(), channelName), NULL);
		return NO_FOUND ;													
	}
	return ALL_RIGHT;
}

/**
 * @brief Checks if the given channel name is valid.
 * 
 * A valid channel name must start with a '#' character and have a length greater than 1.
 * 
 * @param channelName The name of the channel to be validated.
 * @return true if the channel name is valid, false otherwise.
 */
bool IrcHelper::isValidChannelName(const std::string& channelName)
{
	return channelName[0] == '#' && (channelName).length() > 1;
}

/**
 * @brief Ensures that a channel name starts with a '#' character.
 *
 * This function checks if the given channel name starts with a '#' character.
 * If it does not, the function prepends a '#' to the channel name. If it already
 * starts with a '#', the function returns the channel name unchanged.
 *
 * @param channelName The name of the channel to be fixed.
 * @return A string representing the fixed channel name, guaranteed to start with a '#'.
 */
std::string IrcHelper::fixChannelMask(std::string channelName)
{
	std::string res = channelName;
	if (res[0] != '#')
		res = '#' + res;
	return res;
}

/**
 * @brief Checks if a channel exists in the given map of channels.
 * 
 * @param channelName The name of the channel to check for existence.
 * @param channels A map of channel names to Channel pointers.
 * @return true if the channel exists in the map, false otherwise.
 */
bool IrcHelper::channelExists(const std::string& channelName, std::map<std::string, Channel*>& channels)
{
	return channels.find(channelName) != channels.end();
}

/**
 * @brief Checks if a client exists based on the given file descriptor.
 * 
 * @param clientFd The file descriptor of the client.
 * @return true if the client file descriptor is valid (not -1), false otherwise.
 */
bool IrcHelper::clientExists(int clientFd)
{
	return clientFd != -1;
}


// === MODE HELPER ===

/**
 * @brief Checks if the given mode string is in the correct format.
 *
 * The mode string is considered to be in the correct format if:
 * - It is not empty.
 * - It has at least two characters.
 * - The first character is either '+' or '-'.
 * - All subsequent characters are one of the following: 'i', 't', 'k', 'o', 'l', '-', '+'.
 *
 * @param mode The mode string to be checked.
 * @return Returns 0 if the mode string is in the correct format.
 *         Returns -1 if the mode string is empty, has less than two characters, or the first character is not '+' or '-'.
 *         Returns the index of the first invalid character if the mode string contains invalid characters.
 */
int IrcHelper::isRightMode(const std::string &mode)
{
	if (mode.empty() || mode.size() < 2 || (mode[0] != '-' && mode[0] != '+'))
		return -1;
	for (int i = 1; mode[i]; i++)
		if (mode[i] != 'i' && mode[i] != 't' && mode[i] != 'k' && mode[i] != 'o' && mode[i] != 'l' && mode[i] != '-' && mode[i] != '+')
			return i;
	return 0;
}

/**
 * @brief Finds the index of the first occurrence of either target1 or target2 in a string, starting from a given position.
 *
 * This function searches for the first occurrence of either target1 or target2 in the given string, starting from the specified position.
 * If either target1 or target2 is found, the function returns the index of the character. If neither target1 nor target2 is found,
 * the function returns -1. If the start position is greater than or equal to the length of the string, the function also returns -1.
 *
 * @param str The string in which to search for the target characters.
 * @param target1 The first character to search for.
 * @param target2 The second character to search for.
 * @param startPos The position in the string to start searching from.
 * @return The index of the first occurrence of either target1 or target2 in the string, or -1 if neither target is found or if the start position is invalid.
 */
int IrcHelper::findCharBeforeIndex(const std::string& str, char target1, char target2, size_t startPos)
{
	if (startPos >= str.length())
		return -1;  // Si la position de départ est en dehors de la chaîne

	for (int i = static_cast<int>(startPos); i >= 0; i--)
		if (str[i] == target1 || str[i] == target2)
			return i;  // Retourne l'index du caractère trouvé

	return -1;  // Si le caractère n'est pas trouvé
}

/**
 * @brief Determines the number of arguments required based on the given mode string.
 *
 * This function calculates how many arguments are needed by examining the provided mode string.
 * The base number of arguments is 2. Additional arguments are required if certain characters
 * are found in the mode string:
 * - If 'o' is found, one additional argument is required.
 * - If 'l' is found and it is preceded by a '+' (ignoring '-' characters), one additional argument is required.
 * - If 'k' is found and it is preceded by a '+' (ignoring '-' characters), one additional argument is required.
 *
 * @param mode The mode string to be analyzed.
 * @return The total number of arguments required based on the mode string.
 */
size_t IrcHelper::getExpectedArgCount(std::string mode)
{
	size_t size = 2;
	
	if (mode.find('o') != std::string::npos)
		size++;
	if (mode.find('l') != std::string::npos && mode[findCharBeforeIndex(mode, '-', '+', mode.find('l'))] == '+')
		size++;
	if (mode.find('k') != std::string::npos)
		size++;
	return size;
}

/**
 * @brief Parses a vector of arguments to determine which mode corresponds to which argument.
 *
 * This function takes a vector of strings as input, where the first element is expected to be a mode string
 * (e.g., "+o-k+l") and the subsequent elements are the arguments corresponding to those modes. It returns a map
 * where the keys are mode characters ('o', 'k', 'l', etc.) and the values are the corresponding arguments.
 *
 * @param args A vector of strings containing the mode string and its corresponding arguments.
 * @return A map where the keys are mode characters and the values are the corresponding arguments.
 */
std::map<char, std::string> IrcHelper::mapModesToArgs(std::vector<std::string> args)
{
	std::string mode = *++args.begin();
	std::map<char, std::string> mode_args;
	std::vector<std::string>::iterator args_mode_it = ++args.begin();
	args_mode_it++;
	std::map<char, std::string>::iterator it_mode;

	for (size_t i = 0; i < mode.size(); i++)
	{
		if (mode[i] == 'o' || mode[i] == 'k')
		{
			mode_args.insert(std::make_pair(mode[i], *args_mode_it));
			args_mode_it++;
		}
		else if (mode[i] == 'l')
		{
			if (mode[findCharBeforeIndex(mode, '-', '+', i)] == '+')
			{
				mode_args.insert(std::make_pair(mode[i], *args_mode_it));//on met la nouvelle cle + arg qui nous interesse
				args_mode_it++;											//on avance. si on decide de prendre que le 1er, juste a supprimer la conditiom
			}
		}
	}
	return (mode_args);
}

/**
 * @brief Checks for duplicate characters in a string starting from a given index.
 *
 * This function iterates through the string starting from the index `i + 1` and checks if the character `c` appears again.
 * If a duplicate character is found, it throws an `std::invalid_argument` exception with a message indicating the presence of duplicates.
 *
 * @param str The string to be checked for duplicates.
 * @param c The character to check for duplicates.
 * @param i The index from which to start checking (the function starts checking from `i + 1`).
 *
 * @throws std::invalid_argument If a duplicate character is found.
 */
void IrcHelper::assertNoDuplicate(std::string &str, char c, size_t i)
{
	i++;
	while (str[i])
	{
		if (str[i] == c)
			throw std::invalid_argument(MessageBuilder::ircNoticeMsg("doublons", RED));
		i++;
	}
}

/**
 * @brief Checks if there is no change to make based on the mode sign and current mode state.
 *
 * This function determines whether a mode change is necessary based on the provided mode sign
 * and the current state of the mode. If the mode sign is '+' and the mode is not enabled, or
 * if the mode sign is '-' and the mode is enabled, it returns false indicating that a change
 * is needed. Otherwise, it returns true indicating no change is necessary.
 *
 * @param modeSign The sign of the mode change ('+' to enable, '-' to disable).
 * @param modeEnabled The current state of the mode (true if enabled, false if disabled).
 * @return true if no change is necessary, false if a change is needed.
 */
bool IrcHelper::noChangeToMake(char modeSign, bool modeEnabled)
{
	if ((modeSign == '+' && modeEnabled == false)
		|| (modeSign == '-' && modeEnabled == true))
		return false;
	return true;
}

/**
 * @brief Checks if the given limit string is a valid numerical limit.
 *
 * This function verifies that the input string `limit` contains only digits,
 * and that the numerical value it represents is greater than 0 and less than
 * or equal to 2147483647.
 *
 * @param limit A reference to the string representing the limit to be validated.
 * @return true if the limit is a valid numerical value within the specified range, false otherwise.
 */
bool IrcHelper::isValidLimit(std::string &limit)
{
	return (!(limit.find_first_not_of("0123456789") != std::string::npos) && std::atol(limit.c_str()) > 0 && std::atol(limit.c_str()) <= 2147483647);
}