#include "../../incs/classes/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/Utils.hpp"
#include "../../incs/classes/IrcHelper.hpp"
#include "../../incs/classes/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"

// =========================================================================================

// === COMMAND HANDLER ===

/**
 * @brief Processes a command message sent to the bot.
 *
 * This function handles the incoming command message by first checking if it is a valid bot command.
 * It performs the following steps:
 * 1. Checks if the message does not contain a bot command using `_noBotCommandFound`.
 * 2. Handles any special commands using `_handleSpecialCommands`.
 * 3. Parses the private message using `_parsePrivmsg`.
 * 
 * If any of the above steps indicate that the message should not be processed further, the function returns early.
 * Otherwise, it generates a response using `_handleCommand` and sends the response message using `_sendMessage`.
 *
 * @param message The command message to be processed.
 */
void Bot::_manageCommand(std::string& message)
{
	if (_handleSpecialCommands(message)
		|| _noBotCommandFound(message)
		|| !_parsePrivmsg(message))
		return;

	std::string response = _handlePrivmsgCommand();
	_sendMessage(MessageHandler::botCmdPrivmsg(_target, response));
}

/**
 * @brief Handles special IRC messages.
 *
 * This function processes special messages received by the bot. It currently
 * handles PING messages by responding with a PONG message and checks for
 * invite messages.
 *
 * @param input The message received by the bot.
 * @return true if the message was a special message and was handled, false otherwise.
 */
bool Bot::_handleSpecialCommands(const std::string& input)
{
	if (input == MessageHandler::ircPing())
	{
		_sendMessage(MessageHandler::ircPong());
		return true;
	}
	if (_handleInvite(input))
		return true;
	return false;
}

/**
 * @brief Checks if a bot command is found in the input string.
 *
 * This function determines the position of a bot command within the input string
 * using the IrcHelper::getBotCommandStartPos function. If a bot command is found,
 * it returns false. Otherwise, it returns true.
 *
 * @param input The input string to be checked for a bot command.
 * @return true if no bot command is found, false otherwise.
 */
bool Bot::_noBotCommandFound(const std::string& input)
{
	size_t commandPos = IrcHelper::getBotCommandStartPos(input);
	if (commandPos != std::string::npos)
		return false;
	return true;
}

/**
 * @brief Handles an invite message and joins the specified channel.
 *
 * This function processes an invite message, verifies its format, extracts the
 * channel name from the message, and sends a command to join the channel.
 *
 * @param input The invite message to be processed.
 * @return true if the invite message is successfully processed and the join
 *         command is sent; false otherwise.
 */
bool Bot::_handleInvite(const std::string& input)
{
	std::cout << "Handling invite..." << std::endl;
	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() < 4)
		return false;

	// Vérification du format du message
	if (args[0] != ":" + server::NAME || args[1] != "NOTICE" || args[2] != server::NAME)
		return false;

	// Extraction du message d'invitation
	std::string message = Utils::stockVector(args.begin() + 3, args);
	size_t channelPos = message.find("#");
	if (channelPos == std::string::npos)
		return false;

	// Extraction du nom du channel et envoi de la commande JOIN
	std::string channelName = message.substr(channelPos);
	std::cout << "Channel : " << channelName << std::endl;
	_sendMessage(MessageHandler::botCmdJoinChannel(channelName));

	return true;
}