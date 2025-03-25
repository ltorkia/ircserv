#include "../../incs/classes/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/Utils.hpp"
#include "../../incs/classes/IrcHelper.hpp"
#include "../../incs/classes/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/commands.hpp"
#include "../../incs/config/server_messages.hpp"

using namespace server_messages;

// =========================================================================================

// === COMMAND HANDLER SERVER ===

/**
 * @brief Manages the incoming command message for the bot.
 *
 * This function processes the incoming message and determines the appropriate
 * action to take based on the command contained within the message. It handles
 * various commands such as ping, invite, and join. If none of these commands
 * are detected, it parses the message as a private message command and sends
 * the appropriate response.
 *
 * @param message The incoming command message to be managed.
 */
void Bot::_manageServerCommand(std::string& message)
{
	if (_handlePing(message) || _handleInvite(message)
		|| _handleJoin(message) || !_parsePrivmsg(message))
		return;

	std::string response = _handleBotCommand();
	_sendMessage(MessageHandler::botCmdPrivmsg(_target, response));
}

/**
 * @brief Handles the PING command from the IRC server.
 *
 * This function checks if the input message is a PING command. If it is,
 * it responds with a PONG message.
 *
 * @param input The input message to be checked.
 * @return true if the input message is a PING command and a PONG response
 *         was sent, false otherwise.
 */
bool Bot::_handlePing(const std::string& input)
{
	if (input != MessageHandler::ircPing())
		return false;
	_sendMessage(MessageHandler::ircPong());
	return true;
}

/**
 * @brief Handles an invite message and joins the specified channel.
 *
 * This function processes an invite message, extracts the channel name from the message,
 * and sends a JOIN command to the IRC server to join the specified channel. It also sends
 * a message to the channel listing the bot's features.
 *
 * @param input The invite message received from the IRC server.
 * @return true if the invite message was successfully processed and the bot joined the channel, false otherwise.
 */
bool Bot::_handleInvite(const std::string& input)
{
	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() < 4)
		return false;

	// Vérification du format du message
	if (args[0] != ":" + server::NAME
			|| !_isValidCommand(args, commands::NOTICE)
			|| args[2] != server::NAME)
		return false;

	// Extraction du message d'invitation
	std::string message = _extractMessage(args);
	if (message.empty())
		return false;

	// On cherche la position du nom du channel
	size_t channelPos = message.find("#");
	if (channelPos == std::string::npos)
		return false;

	// Extraction du nom du channel, 
	// suppression des caractères non imprimables,
	// et envoi de la commande JOIN
	_target = message.substr(channelPos, message.length() - channelPos);
	_target.erase(std::remove_if(_target.begin(), _target.end(), Utils::isNonPrintableChar), _target.end());
	_sendMessage(MessageHandler::botCmdJoinChannel(_target));

	sleep(1);

	// Envoi d'un message au channel listant les fonctionnalités du bot
	_announceFeaturesOnce();

	return true;
}

/**
 * @brief Handles the JOIN command for the bot.
 *
 * This function processes the input string to handle the JOIN command.
 * It verifies the format of the message, checks if the command is valid,
 * and sends a message to the client who just joined the channel to list
 * the bot's features.
 *
 * @param input The input string containing the command and its arguments.
 * @return true if the JOIN command was successfully handled, false otherwise.
 */
bool Bot::_handleJoin(const std::string& input)
{
	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() < 3)
		return false;

	// Vérification du format du message
	if (args[1] != commands::JOIN)
		return false;

	if (!_extractSenderNick(args.front())
		|| !_isValidCommand(args, commands::JOIN))
		return false;

	_target = _clientNickname;

	// Envoi d'un message au client qui vient de join le channel
	// pour lister les fonctionnalités du bot
	_announceFeaturesOnce();

	return true;
}

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
 * 7. Parses the bot command from the message content.
 *
 * @param input The input string containing the PRIVMSG command.
 * @return true if the PRIVMSG command is successfully parsed and valid, false otherwise.
 */
bool Bot::_parsePrivmsg(std::string& input)
{
	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() < 4)
		return false;

	if (!_extractSenderNick(args.front())
		|| !_isValidCommand(args, commands::PRIVMSG)
		|| !_extractTarget(args))
		return false;

	std::string message = _extractMessage(args);
	if (message.empty())
		return false;

	return _parseBotCommand(message);
}