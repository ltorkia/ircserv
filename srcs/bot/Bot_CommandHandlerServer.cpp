#include "../../incs/classes/bot/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/utils/Utils.hpp"
#include "../../incs/classes/utils/IrcHelper.hpp"
#include "../../incs/classes/utils/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/bot.hpp"
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
 * This function processes an invite message, verifies its format, extracts the
 * channel name, and sends a JOIN command to the server. It also announces the
 * bot's features to the clients in the channel.
 *
 * @param input The invite message received.
 * @return true if the invite message is successfully processed and the bot joins the channel.
 * @return false if the invite message is invalid or processing fails.
 */
bool Bot::_handleInvite(const std::string& input)
{
	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() < 4)
		return false;

	// Vérification du format du message
	if (args[0] != ":" + server::NAME
		|| !_isRightCommand(args, bot::NOTICE_CMD)
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
	_channelName = message.substr(channelPos, message.length() - channelPos);
	_channelName.erase(std::remove_if(_channelName.begin(), _channelName.end(), Utils::isNonPrintableChar), _channelName.end());
	_target = _channelName;
	_sendMessage(MessageHandler::botCmdJoinChannel(_target));

	sleep(1);

	// Envoi d'un message aux clients du channel pour lister les fonctionnalités du bot
	_announceBotFeatures();
	
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
		|| !_isRightCommand(args, commands::JOIN)
		|| !_extractTarget(args))
		return false;

	// Envoi d'un message au client qui vient de join le channel
	// pour lister les fonctionnalités du bot
	_announceBotFeatures();

	return true;
}

/**
 * @brief Parses a PRIVMSG command from the input string.
 *
 * This function processes an input string to determine if it contains a valid
 * PRIVMSG command. It extracts the sender's nickname, verifies the command,
 * extracts the target and message, and then attempts to parse the bot command
 * from the message. If the message does not contain a valid bot command, it
 * announces the bot's features to the client.
 *
 * @param input The input string containing the PRIVMSG command.
 * @return true if the PRIVMSG command is successfully parsed and contains a valid bot command.
 * @return false if the input is invalid, the command is incorrect, the target or message cannot be extracted,
 *         or the message does not contain a valid bot command.
 */
bool Bot::_parsePrivmsg(std::string& input)
{
	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() < 4)
		return false;

	if (!_extractSenderNick(args.front())
		|| !_isRightCommand(args, commands::PRIVMSG)
		|| !_extractTarget(args))
		return false;

	std::string message = _extractMessage(args);
	if (message.empty())
		return false;

	if (!_parseBotCommand(message))
	{
		// Envoi d'un message au client pour lister les fonctionnalités du bot.
		// Il n'est envoyé qu'une fois en message privé.
		if (!_clientNickname.empty() && _target == _clientNickname)
			_announceBotFeatures();
		return false;
	}
	return true;
}