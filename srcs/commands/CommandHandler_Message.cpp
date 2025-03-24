#include "../../incs/classes/CommandHandler.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/Utils.hpp"
#include "../../incs/classes/IrcHelper.hpp"
#include "../../incs/classes/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/commands.hpp"

using namespace commands;
using namespace channel_error;
using namespace error_display;

// =========================================================================================
/**
 * @brief Sends a private message to a specified target (user or channel).
 * 
 * This function handles the PRIVMSG command, which sends a private message to a user or a channel.
 * It validates the input parameters, checks the message format, and sends the message to the appropriate recipients.
 * 
 * @throws std::invalid_argument if the parameters are invalid or if there is no text to send.
 */
void CommandHandler::_sendPrivateMessage()
{
	if (Utils::isEmptyOrInvalid(_itInput, _vectorInput))
		throw std::invalid_argument(MessageHandler::ircNeedMoreParams(_client->getNickname(), PRIVMSG));

	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::SENTENCE);
	std::string targetStr = *args.begin();
	std::vector<std::string> targets = Utils::getTokens(targetStr, splitter::COMMA);
	std::vector<std::string>::iterator itTarget = targets.begin();

	if (itTarget == targets.end())
		throw std::invalid_argument(MessageHandler::ircNeedMoreParams(_client->getNickname(), PRIVMSG));

	std::vector<std::string>::iterator itMessage = ++args.begin();
	std::string	message = itMessage != args.end() ? *itMessage : "";

	if (IrcHelper::isRightChannel(*_client, *itTarget, _channels, HIDE_ERROR) != INVALID_FORMAT)
		_sendToChannel(targets, message);
	else
		_sendToClient(targets, message);
}

/**
 * @brief Sends a message to a list of target channels.
 *
 * This function iterates over the provided list of target channels and sends the given message to each channel.
 * If the message is empty, contains only a colon, or consists solely of whitespace, an exception is thrown.
 * The message is formatted before being sent to the channels.
 *
 * @param targets A vector of target channel names to which the message should be sent.
 * @param message The message to be sent to the target channels.
 *
 * @throws std::invalid_argument if the message is empty, contains only a colon, or consists solely of whitespace.
 */
void CommandHandler::_sendToChannel(std::vector<std::string>& targets, std::string& message)
{
	std::string nickname = _client->getNickname();

	for (std::vector<std::string>::iterator itTarget = targets.begin(); itTarget != targets.end(); itTarget++)
	{
		if (message.empty() || (message[0] == ':' && message.size() == 1) || Utils::isOnlySpace(message) == true)
			throw std::invalid_argument(MessageHandler::ircNoTextToSend(nickname));

		std::string formattedMessage = IrcHelper::sanitizeIrcMessage(message, PRIVMSG, nickname);
		std::string targetName = *itTarget;

		if (IrcHelper::channelExists(targetName, _channels) == false)
		{
			_client->sendMessage(MessageHandler::ircNoSuchChannel(nickname, targetName), NULL);
			continue;
		}
		Channel* channel = _channels[targetName];
		channel->sendToAll(MessageHandler::ircMsgToChannel(nickname, targetName, formattedMessage), _client, false);
	}
}

/**
 * @brief Sends a message to a list of target clients.
 *
 * This function iterates over a list of target client nicknames and sends a formatted message to each one.
 * If the message is empty or invalid, it throws an exception.
 * If the target client is not found, it sends an error message back to the sender.
 * If the target client is the same as the sender, it skips sending the message.
 * If the target client is away, it sends an away message back to the sender.
 *
 * @param targets A vector of target client nicknames.
 * @param message The message to be sent to the target clients.
 * @throws std::invalid_argument If the message is empty or invalid.
 */
void CommandHandler::_sendToClient(std::vector<std::string>& targets, std::string& message)
{
	std::string nickname = _client->getNickname();

	for (std::vector<std::string>::iterator itTarget = targets.begin(); itTarget != targets.end(); itTarget++)
	{
		std::string targetName = *itTarget;
		if (message.empty() || (message[0] == ':' && message.size() == 1) || Utils::isOnlySpace(message) == true)
		{
			if ((targetName)[0] == ':')
				throw std::invalid_argument(MessageHandler::ircNoRecipient(nickname));
			else
				throw std::invalid_argument(MessageHandler::ircNoTextToSend(nickname));
		}

		std::string formattedMessage = IrcHelper::sanitizeIrcMessage(message, PRIVMSG, nickname);
		int clientFd = _server.getClientByNickname(targetName, NULL);
		if (IrcHelper::clientExists(clientFd) == false)
		{
			_client->sendMessage(MessageHandler::ircNoSuchNick(nickname, targetName), NULL);
			continue;
		}
		
		Client* targetClient = _clients[clientFd];
		if (targetClient == _client)
			continue;

		targetClient->sendMessage(MessageHandler::ircMsgToClient(nickname, targetName, formattedMessage), _client);			
		
		// Si le client visé est absent, l'envoyeur reçoit sa notification d'absence
		if (targetClient->isAway())
			_client->sendMessage(MessageHandler::ircClientIsAway(nickname, targetName, targetClient->getAwayMessage()), NULL);
	}
}