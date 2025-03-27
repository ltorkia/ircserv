#include "../../../incs/server/Client.hpp"

// === OTHER CLASSES ===
#include "../../../incs/server/Channel.hpp"
#include "../../../incs/utils/Utils.hpp"
#include "../../../incs/utils/IrcHelper.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/irc_config.hpp"

// =========================================================================================

// === CHANNEL MANAGER ===

// ========================================= PUBLIC ========================================

// === CHANNEL GETTERS ===

std::map<std::string, Channel*>& Client::getChannelsJoined()
{
	return _channelsJoined;
}
bool Client::isInChannel(const std::string& channelName) const
{
	return _channelsJoined.find(channelName) != _channelsJoined.end();
}
bool Client::isOperator(Channel* channel) const
{
	return channel->isOperator(this);
}
bool Client::isInvited(const Channel* channel) const
{
	return channel->isInvited(this);
}

// === CHANNEL ACTIONS ===
/**
 * @brief Joins the client to a specified channel, creating the channel if it does not exist.
 * 
 * This function attempts to join the client to a channel identified by `channelName`. If the channel
 * already exists, the client is added to it. If the channel does not exist, it is created and the client
 * is then added to the newly created channel.
 * 
 * @param channelName The name of the channel to join.
 * @param password The password required to join the channel.
 * @param channels A map of existing channels where the key is the channel name and the value is a pointer to the Channel object.
 */
void Client::joinChannel(const std::string& channelName, const std::string& password, std::map<std::string, Channel*>& channels)
{
	if (!IrcHelper::isValidChannelName(channelName) || !Utils::isOnlyAlphaNum((channelName).substr(1)))
	{
		sendMessage(MessageBuilder::ircBadChannelName(_nickname, channelName), NULL);
		return;
	}

	// Si le canal existe, on peut utiliser it->second (Channel*) pour ajouter le client
	// (si addToChannel() renvoie false, il y a une erreur),
	// sinon on créé d'abord le channel et on ajoute le client ensuite.
	if (IrcHelper::channelExists(channelName, channels))
	{
		std::map<std::string, Channel*>::iterator it = channels.find(channelName); 
		if (!isInChannel(channelName) )
			addToChannel(it->second, password, channelName, channels);
		return;
	}
	createChannel(channelName, password, channels);
}

/**
 * @brief Creates a new channel if it does not already exist.
 *
 * This function attempts to create a new channel with the specified name and password.
 * If the channel already exists, no action is taken. If the channel name is invalid,
 * an error message is sent to the client.
 *
 * @param channelName The name of the channel to be created.
 * @param password The password for the channel. If empty, the channel will have no password.
 * @param channels A map of existing channels where the key is the channel name and the value is a pointer to the Channel object.
 */
void Client::createChannel(const std::string& channelName, const std::string& password, std::map<std::string, Channel*>& channels)
{
	// Si le canal n'existe pas déjà, on le crée et on l'ajoute
	if (!IrcHelper::channelExists(channelName, channels))
	{
		channels[channelName] = new Channel(channelName, password);
		if (!password.empty())
		{
			if (!IrcHelper::isValidPassword(password, false))
			{
				sendMessage(MessageBuilder::ircInvalidPasswordFormat(_nickname, channelName), NULL);
				deleteChannel(channels[channelName], channels);
				return;
			}
			channels[channelName]->setPassword(password);
		}
		std::cout << MessageBuilder::msgClientCreatedChannel(_nickname, channelName, password) << std::endl;
		channels[channelName]->addOperator(this);
		addToChannel(channels[channelName], password, channelName, channels);
	}
}

/**
 * @brief Adds the client to a specified channel if the conditions are met.
 *
 * This function attempts to add the client to the given channel. It first checks if the channel exists
 * and if the client is not already in the channel. If the channel exists and the client is not in it,
 * it verifies the provided password. If the password is correct, the client is added to the channel.
 * The function also handles sending appropriate messages to the client and other channel members.
 *
 * @param channel A pointer to the Channel object to which the client is to be added.
 * @param password The password required to join the channel.
 * @param channelName The name of the channel.
 */
void Client::addToChannel(Channel* channel, const std::string& password, const std::string& channelName, std::map<std::string, Channel*>& channels)
{
	if (!IrcHelper::channelExists(channelName, channels))
	{
		sendMessage(MessageBuilder::ircNoSuchChannel(_nickname, channelName), NULL);
		return;
	}

	if (!isInChannel(channelName))
	{
		if (channel->isInviteOnly() && channels[channelName]->isInvited(this) == false)
		{
			sendMessage(MessageBuilder::ircInviteOnly(getNickname(), channelName), NULL);
			return;
		}

		if (!hasRightPassword(channel, password))
			return;
		if (channel->isFull())
		{
			sendMessage(MessageBuilder::ircChannelFull(this->getNickname(), channel->getName()), NULL);
			return ;
		}
		channel->addClient(this);
		_channelsJoined[channelName] = channel;

		msgAfterJoin(channel, channelName);
	}
}

/**
 * @brief Sends a series of messages to the client and the channel after the client joins a channel.
 *
 * This function performs the following actions:
 * 1. Sends a join message to all clients in the channel.
 * 2. Sends the channel mode to the client.
 * 3. Sends the channel creation time to the client.
 * 4. If the channel has a topic, sends the topic and the topic setter information to the client.
 * 5. Sends the list of nicknames in the channel to the client.
 *
 * @param channel A pointer to the Channel object that the client has joined.
 * @param channelName The name of the channel that the client has joined.
 */
void Client::msgAfterJoin(Channel* channel, const std::string& channelName)
{
	sendToAll(channel, MessageBuilder::ircClientJoinChannel(_usermask, channelName), false);
	sendMessage((MessageBuilder::ircChannelModeIs(_nickname, channelName, channel->getModes())), NULL);
	sendMessage(MessageBuilder::ircCreationTime(_nickname, channelName, channel->getCreationTime()), NULL);

	if (channel->hasTopic())
	{
		sendMessage(MessageBuilder::ircTopic(_nickname, channel->getName(), channel->getTopic()), NULL);
		sendMessage(MessageBuilder::ircTopicWhoTime(_nickname, channel->getTopicSetterMask(), channel->getName(), channel->getTopicTimestamp()), NULL);
	}
	sendMessage(MessageBuilder::ircNameReply(_nickname, channelName, channel->getNicknames()), NULL);
	std::cout << MessageBuilder::msgClientJoinedChannel(_nickname, channelName) << std::endl;
}

/**
 * @brief Checks if the provided password is correct for the given channel.
 *
 * This function verifies if the provided password matches the password of the specified channel.
 * If the channel requires a password and the provided password is empty, it sends a message indicating
 * that a password is required. If the provided password is incorrect, it sends a message indicating
 * that the password is wrong. If the channel does not require a password and a password is provided,
 * it sends a message indicating that no password is needed.
 *
 * @param channel A pointer to the Channel object for which the password is being checked.
 * @param password The password provided by the client.
 * @return true if the password is correct or if the channel does not require a password.
 * @return false if the password is incorrect or if a password is required but not provided.
 */
bool Client::hasRightPassword(Channel* channel, const std::string& password)
{
	if (channel->hasPassword())
	{
		if (password.empty() || (!password.empty() && password != channel->getPassword()))
			sendMessage(MessageBuilder::ircWrongChannelPass(_nickname, channel->getName()), NULL);
		else
			return true;
		return false;
	}
	if (!channel->hasPassword() && !password.empty())
		sendMessage(MessageBuilder::ircNoPassNeeded(_nickname), NULL);
	return true;
}

/**
 * @brief Sets the password for a given channel if the client is an operator.
 *
 * This function checks if the client is an operator of the specified channel.
 * If the client is an operator, it sets the channel's password to the provided value.
 * Regardless of whether the password is set, it sends a message indicating that the client is not a channel operator.
 *
 * @param channel A pointer to the Channel object for which the password is to be set.
 * @param password The new password to be set for the channel.
 */
void Client::passwordSetting(Channel* channel, const std::string& password)
{
	if (isOperator(channel))
	{
		channel->setPassword(password);
		return;
	}
	sendMessage(MessageBuilder::ircNotChanOperator(channel->getName()), NULL);
}

/**
 * @brief Handles the event when a client is kicked from a channel.
 *
 * This function is called when a client is kicked out of a channel by another client.
 * It removes the client from the specified channel and logs the reason for the kick.
 *
 * @param channel The channel from which the client is being kicked.
 * @param kicker The client who is performing the kick.
 * @param reason The reason for which the client is being kicked.
 */
void Client::isKickedFromChannel(Channel *channel, Client* kicker, const std::string& reason)
{
	channel->removeClient(this, kicker, reason, leaving_code::KICKED);
}

/**
 * @brief Handles the invitation of a client to a channel.
 * 
 * This function checks if the client is already in the specified channel.
 * If the client is not in the channel, they are added to the channel's invited list.
 * If the client is already in the channel, a message is sent to the inviter indicating that the client is already in the channel.
 * 
 * @param channel Pointer to the Channel object to which the client is being invited.
 * @param inviter Pointer to the Client object who is sending the invitation.
 */
void Client::isInvitedToChannel(Channel *channel, const Client* inviter)
{	
	if (!channel)
		return;
	if (!isInChannel(channel->getName()))
		channel->addClientToInvitedList(this, inviter);
	else
		inviter->sendMessage(MessageBuilder::ircAlreadyOnChannel(inviter->getNickname(), _nickname, channel->getName()), NULL);
}

/**
 * @brief Removes the client from the specified channel and deletes the channel if it becomes empty.
 * 
 * @param it Iterator pointing to the channel in the channels map.
 * @param channels Reference to the map of channels.
 */
void Client::leaveChannel(std::map<std::string, Channel*>::iterator it, std::map<std::string, Channel*>& channels, const std::string& reason, int reasonCode)
{
	Channel* channel = it->second;
	if (channel && isInChannel(channel->getName()))
	{
		channel->removeClient(this, NULL, reason, reasonCode);

		// Si le canal n'a plus de clients, on le supprime
		deleteChannel(channel, channels);
	}
}

/**
 * @brief Makes the client leave all channels it has joined.
 *
 * This function iterates through all the channels the client has joined and 
 * makes the client leave each one. It uses the leaveChannel function to 
 * handle the process of leaving each channel.
 *
 * @param channels A map of channel names to Channel pointers, representing 
 * all available channels.
 */
void Client::leaveAllChannels(std::map<std::string, Channel*>& channels, const std::string& reason, int reasonCode)
{
	while (!_channelsJoined.empty())
		leaveChannel(_channelsJoined.begin(), channels, reason, reasonCode);
}

/**
 * @brief Deletes a channel if it has no clients.
 *
 * This function checks if the given channel has no clients. If the channel is empty,
 * it removes the channel from the provided map of channels, prints messages indicating
 * that the channel had no clients and that it has been destroyed, and then deletes the channel.
 *
 * @param channel A pointer to the Channel object to be deleted.
 * @param channels A reference to a map of channel names to Channel pointers, from which the channel will be removed.
 */
void Client::deleteChannel(Channel* channel, std::map<std::string, Channel*>& channels)
{
	if (!channel->hasClients())
	{
		std::cout << MessageBuilder::msgNoClientInChannel(channel->getName()) << std::endl;
		std::cout << MessageBuilder::msgChannelDestroyed(channel->getName()) << std::endl;
		channels.erase(channel->getName());
		delete channel;
	}
}