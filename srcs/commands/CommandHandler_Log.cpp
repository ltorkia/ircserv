#include "../../incs/classes/commands/CommandHandler.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/utils/Utils.hpp"
#include "../../incs/classes/utils/IrcHelper.hpp"
#include "../../incs/classes/utils/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/commands.hpp"
#include "../../incs/config/server_messages.hpp"

using namespace commands;
using namespace server_messages;

// =========================================================================================
/**
 * @brief Sends a PONG response to the client.
 * 
 * This function is called when a PING message is received from the client.
 * It sends a PONG message back to the client to acknowledge the PING.
 */
void CommandHandler::_sendPong()
{	
	// Si PING recu, on envoie PONG
	_client->sendMessage(MessageHandler::ircPong(), NULL);
}

/**
 * @brief Updates the activity status of the client.
 * 
 * This function is called to update the client's activity status based on the 
 * reception of a PONG message. If a PONG message is received, it sets the 
 * client's PingSent status to false, indicating that the client is active.
 * 
 * @throws std::invalid_argument if the PONG message is malformed or missing parameters.
 */
void CommandHandler::_updateActivity()
{	
	// Si PONG recu, on set PingSent() à false pour le prochain check d'inactivité du client
	// (l'activite du client a été mise à jour à la réception de cette commande dans handleMessage())
	_client->setPingSent(false);
}

/**
 * @brief Handles the WHO command from the IRC client.
 * 
 * This function processes the WHO command, which can be used to query information
 * about a specific channel or a specific user. The function performs the following steps:
 * 
 * 1. Retrieves the nickname of the requestor.
 * 2. Parses the arguments provided with the WHO command.
 * 3. Validates the number of arguments.
 * 4. If the request is for a channel:
 *    - Checks if the channel exists.
 *    - Retrieves the list of clients in the channel.
 *    - Sends information about each client in the channel to the requestor.
 *    - Sends the end of WHO list message.
 * 5. If the request is for a specific user:
 *    - Checks if the user exists.
 *    - Sends information about the user to the requestor.
 *    - Sends the end of WHO list message.
 * 
 * @throws std::invalid_argument if the number of arguments is invalid or if the requested user/channel does not exist.
 */
void CommandHandler::_handleWho()
{
	std::string requestorNickname = _client->getNickname();
	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::WORD);
	int n_arg = args.size();

	// Si plus de un argument apres split, le format est invalide
	if (n_arg != 1)
		throw std::invalid_argument(MessageHandler::ircNeedMoreParams(requestorNickname, WHO));

	// Si le client demande des infos sur un channel, on vérifie son existence
	// et on affiche les infos de chaque client dans ce channel
	std::string channelName = IrcHelper::fixChannelMask(*_itInput);
	if (IrcHelper::channelExists(channelName, _channels))
	{
		Channel* channel = _channels[channelName];
		std::set<const Client*> clientsList = channel->getClientsList();

		for (std::set<const Client*>::iterator it = clientsList.begin(); it != clientsList.end(); ++it)
		{
			const Client* connected = *it;
			std::string prefix = channel->isOperator(connected) ? "@" : "";
			std::string connectedNickname = (prefix + connected->getNickname());
			_client->sendMessage(MessageHandler::ircWho(requestorNickname, connectedNickname, connected->getUsername(), connected->getRealName(), connected->getClientIp(), channelName, connected->isAway()), NULL);
			if (_client->isAway())
				_client->sendMessage(MessageHandler::ircClientIsAway(requestorNickname, connected->getNickname(), connected->getAwayMessage()), NULL);
		}
		_client->sendMessage(MessageHandler::ircEndOfWho(requestorNickname, channelName), NULL);
		return;
	}

	// Si le client demande des infos sur un utilisateur
	// On vérifie d'abord que l'utilisateur existe
	std::string checkedClientNickname = *_itInput;
	int checkedClientFd = _server.getClientByNickname(checkedClientNickname, NULL);
	if (IrcHelper::clientExists(checkedClientFd) == false)
		throw std::invalid_argument(MessageHandler::ircNoSuchNick(requestorNickname, checkedClientNickname));
	
	const Client* checkedClient = _clients[checkedClientFd];
	_client->sendMessage(MessageHandler::ircWho(requestorNickname, checkedClient->getNickname(), checkedClient->getUsername(), checkedClient->getRealName(), checkedClient->getClientIp(), "*", checkedClient->isAway()), NULL);
	if (_client->isAway())
		_client->sendMessage(MessageHandler::ircClientIsAway(requestorNickname, checkedClient->getNickname(), checkedClient->getAwayMessage()), NULL);
	_client->sendMessage(MessageHandler::ircEndOfWho(requestorNickname, "*"), NULL);
}

/**
 * @brief Handles the WHOIS command.
 * 
 * This function processes the WHOIS command, which is used to query information about a specific user.
 * It validates the input, checks if the requested nickname exists, and sends the appropriate WHOIS response.
 * Returns if the input is empty, invalid.
 * 
 * @throws if the requested nickname does not exist.
 */
void CommandHandler::_handleWhois()
{
	std::string requestorNickname = _client->getNickname();
	if (Utils::isEmptyOrInvalid(_itInput, _vectorInput))
		return;

	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::WORD);
	int n_arg = args.size();

	// Si plus de un argument apres split, le format est invalide
	if (n_arg != 1)
		throw std::invalid_argument(MessageHandler::ircNeedMoreParams(requestorNickname, WHOIS));
	
	std::string checkedClientNickname = *_itInput;
	int checkedClientFd = _server.getClientByNickname(checkedClientNickname, NULL);
	if (IrcHelper::clientExists(checkedClientFd) == false)
		throw std::invalid_argument(MessageHandler::ircNoSuchNick(requestorNickname, checkedClientNickname));
	
	const Client* checkedClient = _clients[checkedClientFd];
	_client->sendMessage(MessageHandler::ircWhois(requestorNickname, checkedClient->getNickname(), checkedClient->getUsername(), checkedClient->getRealName(), checkedClient->getClientIp()), NULL);
	_client->sendMessage(MessageHandler::ircWhoisIdle(requestorNickname, checkedClient->getNickname(), checkedClient->getIdleTime(), checkedClient->getSignonTime()), NULL);
	_client->sendMessage(MessageHandler::ircEndOfWhois(requestorNickname, checkedClient->getNickname()), NULL);
}

/**
 * @brief Handles the WHOWAS command.
 *
 * This function sends the end of WHOWAS message to the client.
 * It uses the client's nickname and the current iterator value (_itInput)
 * to construct the message.
 */
void CommandHandler::_handleWhowas()
{
	_client->sendMessage(MessageHandler::ircEndOfWhowas(_client->getNickname(), *_itInput), NULL);
}

/**
 * @brief Handles the AWAY command for setting or unsetting the away status of a client.
 *
 * This function checks if the client should be set to away mode or returned to active mode
 * based on the provided parameters. If no valid parameter is provided and the client is
 * currently away, the client is set back to active mode. If a valid away message is provided,
 * the client is set to away mode with the specified message.
 *
 * The function performs the following steps:
 * 1. Retrieves the client's nickname.
 * 2. Checks if the parameters are empty or invalid and if the client is currently away.
 *    If true, the client is set back to active mode and an un-away message is sent.
 * 3. Sanitizes and truncates the provided away message.
 * 4. If the away message is an empty string or contains just "", the client is set back to active mode.
 * 5. If the away message is valid, the client is set to away mode with the specified message.
 * 6. Sends the appropriate away or un-away message to the client.
 */
void CommandHandler::_setAway()
{
	std::string nickname = _client->getNickname();

	// Si pas de paramètre ou invalide et que le client est déjà en mode actif, on ignore
	// Sinon on le remet en mode actif
	if (Utils::isEmptyOrInvalid(_itInput, _vectorInput))
	{
		if (_client->isAway() == true)
		{
			_client->setIsAway(false);
			_client->setAwayMessage("");
			_client->sendMessage(MessageHandler::ircUnAway(nickname), NULL);
		}
		return;
	}

	// On récupère la raison de l'absence
	std::string awayMessage = Utils::truncateStr(IrcHelper::sanitizeIrcMessage(*_itInput, AWAY, nickname));

	// Si la raison contient juste "", on le remplace par une chaine vide pour remettre en mode actif
	if (awayMessage == "\"\"")
		awayMessage = "";

	// Si la raison est vide et que le client est déjà en mode actif, on ignore
	// sinon on le remet en mode actif
	if (awayMessage.empty())
	{
		if (_client->isAway() == true)
		{
			_client->setIsAway(false);
			_client->setAwayMessage("");
			_client->sendMessage(MessageHandler::ircUnAway(nickname), NULL);
		}
		return;
	}
	_client->setIsAway(true);
	_client->setAwayMessage(awayMessage);
	_client->sendMessage(MessageHandler::ircAway(nickname), NULL);
}

/**
 * @brief Handles the QUIT command for a client, preparing them to leave the server.
 *
 * This function checks if a reason for quitting is provided. If no reason is given,
 * or if the reason is empty or consists only of spaces, or if the reason is a single
 * colon, the default reason is used. If a reason is provided, it is checked and
 * truncated if necessary. If the reason is "leaving", it is replaced with the default
 * reason.
 *
 * @note The default reason is defined by the constant DEFAULT_REASON.
 */
void CommandHandler::_quitServer(void)
{
	// Si aucune raison n'est fournie, on utilise la raison par défaut
	if (Utils::isEmptyOrInvalid(_itInput, _vectorInput) || ((*_itInput)[0] == ':' && (*_itInput).size() == 1)) {
		_server.prepareClientToLeave(_it, DEFAULT_REASON);
		return;
	}

	// On récupère la raison du QUIT,
	// si c'est celle par défaut d'Irssi (leaving) on met la notre (Bye bye everyone)
	std::string reason = Utils::truncateStr(IrcHelper::sanitizeIrcMessage(*_itInput, QUIT, _client->getNickname()));
	if (reason == "leaving")
		reason = DEFAULT_REASON;

	// Le client quitte le serveur
	_server.prepareClientToLeave(_it, reason);
}