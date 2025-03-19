#include "../../incs/classes/CommandHandler.hpp"

// === NAMESPACES ===
using namespace commands;
using namespace server_messages;

// =========================================================================================
//-------------------------------manage_command channel -----------------------------------------------------

/**
 * @brief Handles the INVITE command to invite a client to a channel.
 * 
 * This function processes the INVITE command by performing the following steps:
 * 1. Tokenizes the input arguments.
 * 2. Validates the number of arguments.
 * 3. Checks if the client to be invited exists on the server.
 * 4. Verifies the existence of the specified channel.
 * 5. Ensures the requesting client is a member of the specified channel.
 * 6. Checks if the requesting client has operator privileges if the channel is invite-only.
 * 7. Invites the specified client to the channel if all checks pass.
 * 
 * @throws std::invalid_argument if the number of arguments is invalid.
 * @throws std::invalid_argument if the client to be invited does not exist.
 * @throws std::invalid_argument if the specified channel does not exist.
 * @throws std::invalid_argument if the requesting client is not in the specified channel.
 * @throws std::invalid_argument if the requesting client is not an operator in an invite-only channel.
 */
void CommandHandler::_inviteChannel()
{
	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::WORD);
	int n_arg = args.size();

	// Si plus de deux arguments apres split, le format est invalide
	if (n_arg == 0 || n_arg > 2)
		throw std::invalid_argument(MessageHandler::ircNeedMoreParams(_client->getNickname(), INVITE));
	
	std::string invitedName = *args.begin();

	// Verifie que le channel existe
	std::vector<std::string>::iterator itChannel = ++args.begin();
	std::string channelName = itChannel != args.end() ? IrcHelper::fixChannelMask(*itChannel) : "";

	if (IrcHelper::channelExists(channelName, _channels) == false) 
		throw std::invalid_argument(MessageHandler::ircNoSuchChannel(_client->getNickname(), channelName));
		
	// Verifie l'existence du client sur le serveur, si non retourne -1
	int invitedClientFd = _server.getClientByNickname(invitedName, _client);
	if (IrcHelper::clientExists(invitedClientFd) == false)
		throw std::invalid_argument(MessageHandler::ircNoSuchNick(_client->getNickname(), invitedName));

	Channel* channel = _channels[channelName];
	Client* invitedClient = _clients[invitedClientFd];

	// Verifie que le client qui fait la demande est bien dans le channel concerne
	if (!_client->isInChannel(channelName))
		throw std::invalid_argument(MessageHandler::ircCurrentNotInChannel(_client->getNickname(), channel->getName())); 
	
	// Verifie que si le mode "+i" est present, le client faisant la requete est bien operator
	if (channel->getInvites() && channel->isOperator(_client) == false)
		throw std::invalid_argument(MessageHandler::ircNotChanOperator(channelName));
	
	// La fonction isInvitedToChannel() verifie que le client est deja dans le channel avant de l'inviter
	invitedClient->isInvitedToChannel(channel, _client);
}

/**
 * @brief Handles the process of joining channels for a client.
 * 
 * This function parses the input to extract the channels to join and their associated passwords (if any).
 * It then iterates over the list of channels and attempts to join each one using the provided password.
 * 
 * The input is expected to be in the format:
 * "<channel1,channel2,...> <password1,password2,...>"
 * 
 * If no passwords are provided, it will attempt to join the channels without passwords.
 * If a password is "x", it will be treated as an empty password.
 * 
 * @note The function assumes that the input tokens are separated by spaces and commas.
 */
void CommandHandler::_joinChannel()
{	
	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::WORD);

	// Si plus de deux arguments apres split, le format est invalide
	if (args.size() > 2)
		throw std::invalid_argument(MessageHandler::ircNeedMoreParams(_client->getNickname(), JOIN));

	// On recupere les channels a join
	std::vector<std::string>::iterator itArg = args.begin();
	std::vector<std::string> channelsToJoin = Utils::getTokens(*itArg, splitter::COMMA);
	++itArg;

	// On recupere les mots de passe associes aux channels s'il y en a
	std::vector<std::string> passwords = (itArg != args.end()) ? Utils::getTokens(*itArg, splitter::COMMA) : std::vector<std::string>();
	std::vector<std::string>::iterator itPassword = passwords.begin();

	// On boucle sur tous les channels a join
	for (std::vector<std::string>::iterator itChannel = channelsToJoin.begin(); itChannel != channelsToJoin.end(); ++itChannel)
	{
		std::string channelName = *itChannel;
		std::string password = (itPassword != passwords.end()) ? *itPassword : "";
		password = password == "x" ? "" : password;

		_client->joinChannel(channelName, password, _channels);
		if (itPassword != passwords.end())
			++itPassword;
	}
}

/**
 * @brief Sets the topic of a channel.
 * 
 * This function handles the setting of a new topic for a specified channel. 
 * If no new topic is provided, it sends the current topic of the channel to the client.
 * 
 * @throws std::invalid_argument if the channel does not exist or if the client is not authorized to change the topic.
 * 
 * The function performs the following steps:
 * 1. Parses the command arguments to extract the channel name and the new topic.
 * 2. Validates the existence of the channel.
 * 3. If no new topic is provided, sends the current topic to the client.
 * 4. Checks the format of the new topic.
 * 5. If the new topic is an empty string, it unsets the current topic.
 * 6. If the new topic is the same as the current topic, no action is taken.
 * 7. If the channel has mode +t active, verifies if the client has the rights to change the topic.
 * 8. Sets the new topic, updates the topic setter information and timestamp.
 * 9. Sends the new topic to the client and broadcasts it to all channel members.
 */
void CommandHandler::_setTopic()
{
	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::SENTENCE);
	std::vector<std::string>::iterator itChannel = args.begin();
	std::string channelName = itChannel != args.end() ? IrcHelper::fixChannelMask(*itChannel) : "";

	if (IrcHelper::channelExists(channelName, _channels) == false) 
		throw std::invalid_argument(MessageHandler::ircNoSuchChannel(_client->getNickname(), channelName));

	Channel* channel = _channels[channelName];

	// Si pas de nouveau topic en argument, on send le topic actuel du channel
	std::vector<std::string>::iterator itTopic = ++args.begin();
	if (itTopic == args.end())
	{
		_client->sendMessage(MessageHandler::ircTopicMessage(_client->getUsermask(), channelName, channel->getTopic()), NULL);
		return;
	}

	std::string newTopic = Utils::truncateStr(IrcHelper::sanitizeIrcMessage(*itTopic, TOPIC, _client->getNickname()));

	// Si le nouveau topic contient juste "", on le remplace par une chaine vide pour unset le topic
	if (newTopic == "\"\"")
		newTopic = "";

	// Si le topic est le meme que l'ancien, on ne fait rien
	if ((!channel->getTopic().empty() && newTopic == channel->getTopic())
		|| (channel->getTopic().empty() && newTopic.empty()))
		return;

	// Protection contre les clients non autorisés à changer le topic si mode +t actif
	if ((channel->getRightsTopic() && channel->isOperator(_client) == false))
		throw std::invalid_argument(MessageHandler::ircNotChanOperator(channelName));
	
	// On set le nouveau topic et on send les RPL correspondants
	channel->topicSettings(newTopic, _client);
	channel->sendToAll(MessageHandler::ircTopicMessage(_client->getUsermask(), channelName, channel->getTopic()), _client, true);
	std::cout << MessageHandler::msgClientSetTopic(_client->getNickname(), channelName, channel->getTopic()) << std::endl;
}

/**
 * @brief Handles the KICK command to remove one or more users from a channel.
 * 
 * This function processes the KICK command, which removes specified users from a given channel.
 * It performs several checks to ensure the command is valid and the user issuing the command has
 * the necessary permissions.
 * 
 * @throws std::invalid_argument if the command does not have enough parameters.
 * @throws std::runtime_error if the user issuing the command is not an operator of the channel.
 * 
 * The function performs the following steps:
 * 1. Parses the command arguments to extract the channel name and the list of users to be kicked.
 * 2. Checks if the channel exists and if the user issuing the command is an operator of the channel.
 * 3. Iterates over the list of users to be kicked, checking if each user is in the channel.
 * 4. If the user to be kicked is an operator, they are not kicked.
 * 5. Kicks the user from the channel and sends the appropriate messages.
 */
void CommandHandler::_kickChannel()
{
	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::SENTENCE);
	std::vector<std::string>::iterator itArg = args.begin();

	// On récupère le nom du channel
	std::string channelName = *itArg;
	++itArg;
	if (itArg == args.end())
		throw std::invalid_argument(MessageHandler::ircNeedMoreParams(_client->getNickname(), KICK));

	args = Utils::getTokens(*itArg, splitter::WORD);
	itArg = args.begin();
	
	// On récupère les noms des clients à kicker
	std::vector<std::string> kickedClients = Utils::getTokens(*itArg, splitter::COMMA);
	++itArg;

	// On récupère la raison du kick s'il y en a une, sinon on la parametre par defaut
	std::string reason = Utils::stockVector(itArg, args);
	reason = Utils::truncateStr(IrcHelper::sanitizeIrcMessage(reason, KICK, _client->getNickname()));
	if (reason.empty() || Utils::isOnlySpace(reason) == true || ((reason)[0] == ':' && (reason).size() == 1))
		reason = DEFAULT_KICK_REASON;
	
	// Si le channel n'existe pas, on throw une erreur
	channelName = IrcHelper::fixChannelMask(channelName);
	if (IrcHelper::channelExists(channelName, _channels) == false) 
		throw std::invalid_argument(MessageHandler::ircNoSuchChannel(_client->getNickname(), channelName));

	Channel* channel = _channels[channelName];

	// On boucle sur tous les clients a kick
	for (std::vector<std::string>::iterator itClient = kickedClients.begin(); itClient != kickedClients.end(); itClient++)
	{
		// On vérifie que le client qui kick est bien operator du channel
		if (!_client->isOperator(channel))
			throw std::runtime_error(MessageHandler::ircNotChanOperator(channelName));
		
		// On récupère le fd du client à kick et on vérifie s'il est dans le channel,
		// si non erreur et on passe au client suivant
		std::string kickedNickname = *itClient;
		int clientFd = channel->getChannelClientByNickname(kickedNickname, NULL);
		if (IrcHelper::clientExists(clientFd) == false)
		{
			_client->sendMessage(MessageHandler::ircNotInChannel(_client->getNickname(), channelName, kickedNickname), NULL);
			continue;
		}

		Client* kickedClient = _clients[clientFd];

		// Si le client à kicker est operator, on ne le kick pas
		if (kickedClient->isOperator(channel))
			continue;

		// On kick le client
		kickedClient->isKickedFromChannel(channel, _client, reason);
	}
}

/**
 * @brief Handles the quitting of channels by a client.
 * 
 * This function processes the command to quit one or more channels. It retrieves the list of channels to quit
 * and the reason for quitting (if provided). If no reason is provided, a default reason is used. The function
 * then iterates over the list of channels to quit, checks if each channel exists, and if so, processes the
 * client's departure from the channel.
 * 
 * @details
 * - The channels to quit are extracted from the command arguments.
 * - The reason for quitting is determined, with a default reason used if none is provided.
 * - For each channel to quit, the function checks if the channel exists.
 * - If the channel exists, the client is removed from the channel with the specified reason.
 * - If the channel does not exist, an error message is sent to the client.
 */
void CommandHandler::_quitChannel()
{
	// On récupère les channels à quitter
	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::SENTENCE);
	std::vector<std::string>::iterator itArg = args.begin();
	std::vector<std::string> channelsToQuit = Utils::getTokens(*itArg, splitter::COMMA);
	++itArg;

	// On récupère la raison du PART s'il y en a une, sinon on la parametre par defaut
	std::string reason;

	if (Utils::isEmptyOrInvalid(itArg, args) || ((*itArg)[0] == ':' && (*itArg).size() == 1))
		reason = DEFAULT_REASON;
	else
		reason = Utils::truncateStr(IrcHelper::sanitizeIrcMessage(*itArg, PART, _client->getNickname()));
	
	// On boucle sur tous les channels à quitter
	// On vérifie que le channel existe et on le quitte
	for (std::vector<std::string>::iterator itChanToQuit = channelsToQuit.begin(); itChanToQuit != channelsToQuit.end(); itChanToQuit++) //verifie s ils existent bien avant de quit chaque channel
	{
		std::string channelNameToQuit = IrcHelper::fixChannelMask(*itChanToQuit);
		if (IrcHelper::channelExists(channelNameToQuit, _channels) == false)
			_client->sendMessage(MessageHandler::ircNoSuchChannel(_client->getNickname(), channelNameToQuit), NULL);
		else
			_client->leaveChannel(_channels.find(channelNameToQuit), _channels, reason, leaving_code::LEFT);
	}
}