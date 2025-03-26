#include "../../incs/bot/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/utils/Utils.hpp"
#include "../../incs/utils/IrcHelper.hpp"

// === NAMESPACES ===
#include "../../incs/config/bot_config.hpp"
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/commands.hpp"

using namespace bot_config;

// =========================================================================================

// === PARSING HELPER ===

/**
 * @brief Extracts the sender's nickname from the given string.
 *
 * This function processes the input nickname string by removing the leading
 * colon character (':') if present. It then assigns the processed nickname
 * to the _clientNickname member variable and prints it to the standard output.
 *
 * @param nickname A reference to the string containing the sender's nickname.
 *                 This string may start with a colon character (':').
 * @return true if the extracted nickname is not empty and is different from
 *         the server's name; false otherwise.
 */
bool Bot::_extractSenderNick(std::string& nickname)
{
	if (!nickname.empty() && nickname[0] == ':')
		nickname.erase(0, 1);
	_clientNickname = nickname;

	if (_clientNickname.empty() || _clientNickname == server::NAME)
		return false;

	_target = _clientNickname;
	return true;
}

/**
 * @brief Checks if the given command matches the expected command.
 * 
 * This function compares the second element of the args vector with the provided command string.
 * 
 * @param args A vector of strings containing the command arguments.
 * @param command The command string to be validated against.
 * @return true if the second element of args matches the command string, false otherwise.
 */
bool Bot::_isRightCommand(const std::vector<std::string>& args, const std::string& command)
{
	return args[1] == command;
}

/**
 * @brief Extracts the target from the given arguments.
 * 
 * This function processes the provided vector of strings to extract the target
 * information required by the bot. The target is typically a specific user or
 * channel that the bot needs to interact with.
 * 
 * @param args A vector of strings containing the arguments from which the target
 *             needs to be extracted.
 * @return true if the target was successfully extracted, false otherwise.
 */
bool Bot::_extractTarget(const std::vector<std::string>& args)
{
	_channelName = (args[2] != _botNick) ? args[2] : "";
	_target = !_channelName.empty() ? _channelName : _clientNickname;

	if (_target.empty())
		return false;

	return true;
}

/**
 * @brief Extracts a message from a vector of arguments.
 *
 * This function takes a vector of strings as input and extracts a message
 * starting from the fourth element. If the extracted message starts with a
 * colon (':'), the colon is removed. The resulting message is then printed
 * to the standard output and returned.
 *
 * @param args A reference to a vector of strings containing the arguments.
 * @return A string containing the extracted message.
 */
std::string Bot::_extractMessage(std::vector<std::string>& args)
{
	std::vector<std::string>::iterator itArg = args.begin() + 3;
	std::string message = Utils::stockVector(itArg, args);
	if (!message.empty() && message[0] == ':')
		message.erase(0, 1);
	return message;
}

/**
 * @brief Checks if a bot command is found in the input string.
 *
 * This function searches for the start position of a bot command within the given input string.
 * If a bot command is found, it updates the _commandPos member variable with the position of the command.
 *
 * @param input The input string to search for a bot command.
 * @return true if a bot command is found, false otherwise.
 */
bool Bot::_botCommandFound(const std::string& input)
{
	_commandPos = IrcHelper::getBotCommandStartPos(input);
	if (_commandPos == std::string::npos)
		return false;
	return true;
}

/**
 * @brief Parses a bot command from the given message.
 *
 * This function extracts the command and its potential arguments from the message.
 * It verifies if the command is valid and, if the command is "AGE", it stores the argument.
 *
 * @param message The message containing the bot command to be parsed.
 * @return true if the command is successfully parsed and valid, false otherwise.
 */
bool Bot::_parseBotCommand(std::string& message)
{
	// On extrait la commande et ses potentiels arguments du message
	message = message.substr(_commandPos);
	std::vector<std::string> argsInput = Utils::getTokens(message, splitter::SENTENCE);
	if (argsInput.empty())
		return false;

	// On vérifie que la commande stockée est valide
	_command = argsInput.front();
	if (IrcHelper::isInvalidBotCommand(_command))
		return false;

	// Si la commande est AGE, un argument est attendu, on stocke le reste de la string
	if (_command == AGE_CMD && argsInput.size() > 1)
		_ageArg = argsInput[1];

	return true;
}