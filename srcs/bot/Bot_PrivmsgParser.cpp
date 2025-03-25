#include "../../incs/classes/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/Utils.hpp"
#include "../../incs/classes/IrcHelper.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/commands.hpp"

// =========================================================================================

// === PRIVMSG PARSER ===

/**
 * @brief Parses a PRIVMSG command from the input string.
 *
 * This function processes the input string to extract and validate the components
 * of an IRC PRIVMSG command. It performs the following steps:
 * 1. Tokenizes the input string into arguments.
 * 2. Validates the number of arguments.
 * 3. Extracts and validates the sender's nickname.
 * 4. Validates the PRIVMSG command structure.
 * 5. Extracts the target of the message.
 * 6. Extracts the message content.
 * 7. Parses and executes the bot command from the message content.
 *
 * @param input The input string containing the PRIVMSG command.
 * @return true if the PRIVMSG command is successfully parsed and valid, false otherwise.
 */
bool Bot::_parsePrivmsg(std::string& input)
{
	std::cout << "Parsing input..." << std::endl;
	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() < 4)
		return false;

	Utils::printVector(args);

	if (!_extractSenderNick(args.front())
		|| !_isValidPrivmsg(args)
		|| !_extractTarget(args))
		return false;

	std::string message = _extractMessage(args);
	if (message.empty())
		return false;

	return _parseBotCommand(message);
}

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

	std::cout << "Nick : " << _clientNickname << std::endl;
	return !_clientNickname.empty() && _clientNickname != server::NAME;
}

/**
 * @brief Checks if the given arguments represent a valid PRIVMSG command.
 *
 * This function verifies whether the second element in the provided vector
 * of arguments matches the PRIVMSG command.
 *
 * @param args A vector of strings containing the command arguments.
 * @return true if the second argument is the PRIVMSG command, false otherwise.
 */
bool Bot::_isValidPrivmsg(const std::vector<std::string>& args)
{
	return args[1] == commands::PRIVMSG;
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

	std::cout << "target : " << _target << std::endl;
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

	std::cout << "Message : " << message << std::endl;
	return message;
}

/**
 * @brief Parses a bot command from the given message.
 *
 * This function extracts the bot command from the provided message string.
 * It checks if the command is valid and extracts any necessary arguments.
 *
 * @param message The message string containing the bot command.
 * @return true if a valid bot command is found and parsed, false otherwise.
 */
bool Bot::_parseBotCommand(std::string& message)
{
	_commandPos = IrcHelper::getBotCommandStartPos(message);
	if (_commandPos == std::string::npos || (_commandPos > 0 && !isspace(message[_commandPos - 1])))
		return false;

	message = message.substr(_commandPos);
	std::vector<std::string> argsInput = Utils::getTokens(message, splitter::SENTENCE);
	if (argsInput.empty())
		return false;

	_command = argsInput.front();
	if (IrcHelper::isInvalidBotCommand(_command))
		return false;

	std::cout << "Commande : " << _command << std::endl;

	if (_command == bot::AGE_CMD && argsInput.size() > 1)
	{
		_ageArg = argsInput[1];
		std::cout << "Arguments : " << _ageArg << std::endl;
	}
	return true;
}