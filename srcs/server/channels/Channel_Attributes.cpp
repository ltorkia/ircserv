/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel_Attributes.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:33:03 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

// === OTHER CLASSES ===
#include "Client.hpp"
#include "MessageBuilder.hpp"
#include "IrcHelper.hpp"
#include "bot_config.hpp"

using namespace bot_config;

// =========================================================================================

// === CHANNEL ATTRIBUTES ===

// ========================================= PUBLIC ========================================

// === CHANNEL ATTRIBUTES SETTERS ===

// === MODES SETTINGS ===

void Channel::setPassword(const std::string &password)
{
	_password = password;
}
void Channel::setClientsLimit(const int state)
{
	_clientsLimit = state;
}
void Channel::setInviteOnly(const bool state)
{
	_isInviteOnly = state;
}
void Channel::setSettableTopic(const bool state)
{
	_isSettableTopic = state;
}


// === TOPIC SETTINGS ===

/**
 * @brief Sets the topic of the channel and updates the topic metadata.
 *
 * This function sets the topic of the channel to the provided topic string,
 * updates the metadata related to the topic such as the setter's information
 * and the timestamp, and sends appropriate messages to the setter if the topic
 * is not empty.
 *
 * @param topic The new topic to be set for the channel.
 * @param setter The client who is setting the topic. The client's username,
 *               nickname, and IP address are used to update the topic metadata.
 */
void Channel::topicSettings(const std::string& topic, const Client* setter)
{
	setTopic(topic);
	setTopicSetterMask(setter->getUsermask());
	setTopicTimestamp();

	if (!topic.empty())
	{
		setter->sendMessage(MessageBuilder::ircTopic(setter->getNickname(), _name, _topic), NULL);
		setter->sendMessage(MessageBuilder::ircTopicWhoTime(setter->getNickname(), _topicSetterMask, _name, _topicTimestamp), NULL);
	}
}
void Channel::setTopic(const std::string &topic)
{
	_topic = topic;
}
void Channel::setTopicSetterMask(const std::string& setterUsermask)
{
	_topicSetterMask = setterUsermask;
}
void Channel::setTopicTimestamp()
{
	_topicTimestamp = time(0);
}

// =========================================================================================

// === CHANNEL ATTRIBUTES GETTERS ===

// === CHANNEL INFOS ===

time_t Channel::getCreationTime() const
{
	return _channelTimestamp;
}
const std::string& Channel::getName() const
{
	return _name;
}
/**
 * @brief Retrieves the current mode settings of the channel as a formatted string.
 * 
 * This function constructs a string representing the current mode settings
 * of the channel. The mode settings include:
 * - Invite-only status (+i or -i)
 * - Topic protection status (+t or -t)
 * - Password protection status (+k or -k)
 * - User limit status (+l <limit> or -l)
 * 
 * @return A string representing the current modes of the channel.
 */
std::string Channel::getModes() const
{
	std::stringstream stream;
	stream << (isInviteOnly() ? "+i" : "-i");
	stream << (isSettableTopic() ? " +t" : " -t");
	stream << (hasPassword() ? " +k" : " -k");
	if (hasClientsLimit())
		stream << " +l " << getClientsLimit();
	else
		stream << " -l";
	return stream.str();
}


// === MODES CHECK ===

bool Channel::hasPassword() const
{
	return !_password.empty();
}
bool Channel::hasClientsLimit() const
{
	return _clientsLimit != -1;
}
bool Channel::isInviteOnly() const
{
	return _isInviteOnly;
}
bool Channel::isSettableTopic() const
{
	return _isSettableTopic;
}


// === MODES VALUES ===

const std::string& Channel::getPassword() const
{
	return _password;
}
int Channel::getClientsLimit() const
{
	return _clientsLimit;
}


// === TOPIC CHECK + GETTERS ===

bool Channel::hasTopic() const
{
	return !_topic.empty();
}
const std::string& Channel::getTopic() const
{
	return _topic;
}
const std::string& Channel::getTopicSetterMask() const
{
	return _topicSetterMask;
}
time_t Channel::getTopicTimestamp() const
{
	return _topicTimestamp;
}


// === CLIENTS CHECK ===

bool Channel::isFull() const
{
	return getConnectedCount() + 1 > _clientsLimit && hasClientsLimit();
}
bool Channel::hasClients() const
{
	return !_connected.empty();
}
bool Channel::hasOperators() const
{
	return !_operators.empty();
}
bool Channel::hasInvitedClients() const
{
	return !_invited.empty();
}


// === CLIENTS LISTS ===

std::set<const Client*> Channel::getClientsList() const
{
	return _connected;
}
std::set<const Client*> Channel::getInvitedList() const
{
	return _invited;
}
std::set<const Client*> Channel::getOperatorsList() const
{
	return _operators;
}

int Channel::getConnectedCount() const
{
	return _connected.size();
}
/**
 * @brief Retrieves the file descriptor of a client in the channel by their nickname.
 *
 * This function iterates through the set of connected clients in the channel and
 * checks if the provided nickname matches any client's nickname, excluding the
 * current client. If a match is found, the file descriptor of the matched client
 * is returned.
 *
 * @param nickname The nickname of the client to search for.
 * @param currClient A pointer to the current client to be excluded from the search.
 * @return The file descriptor of the matched client, or -1 if no match is found.
 */
int Channel::getChannelClientByNickname(const std::string &nickname, const Client* currClient)
{
	for (std::set<const Client*>::iterator it = _connected.begin(); it != _connected.end(); it++)
	{
		if (currClient && currClient == *it)
			continue;
		if (nickname == (*it)->getNickname()) 
			return (*it)->getFd();
	}
	return -1;
}
/**
 * @brief Retrieves a space-separated string of nicknames of all clients connected to the channel.
 *
 * This function iterates through the set of connected clients and constructs a string
 * containing their nicknames. If a client is an operator, their nickname is prefixed with "@".
 *
 * @return A std::string containing the nicknames of all connected clients, separated by spaces.
 */
std::string Channel::getNicknames() const
{
	std::string nicknames;
	std::set<const Client*>::iterator it = _connected.begin();
	std::set<const Client*>::iterator end = _connected.end();

	std::string prefix;

	while (it != end)
	{
		prefix = isOperator(*it) ? "@" : "";
		nicknames += (prefix + (*it)->getNickname());
		++it;
		if (it != end)
			nicknames += " ";
	}
	return nicknames;
}

bool Channel::isConnected(const Client* client) const
{
	return _connected.find(client) != _connected.end();
}
bool Channel::isOperator(const Client* client) const
{
	return _operators.find(client) != _operators.end();
}
bool Channel::isInvited(const Client* client) const
{
	return _invited.find(client) != _invited.end();
}