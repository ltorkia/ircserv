#include "../../../incs/server/Channel.hpp"

// === OTHER CLASSES ===
#include "../../../incs/server/Client.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// =========================================================================================

// === CHANNEL PROPERTIES ===

// ========================================= PUBLIC ========================================

// === SETTERS ===

void Channel::setPassword(const std::string &password)
{
	_password = password;
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

void Channel::setInvites(const bool info)
{
	_invites = info;
}
void Channel::setRightsTopic(const bool info)
{
	_rightsTopic = info;
}
void Channel::setLimits(const int info)
{
	_limits = info;
}


// === GETTERS ===

const std::string& Channel::getName() const
{
	return _name;
}
const std::string& Channel::getPassword() const
{
	return _password;
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
time_t Channel::getCreationTime() const
{
	return _channelTimestamp;
}
/**
 * @brief Retrieves the mode settings of the channel as a string.
 *
 * This function constructs a string representing the current mode settings
 * of the channel. The mode settings include:
 * - Invite-only status (+i or -i)
 * - Topic protection status (+t or -t)
 * - Password protection status (+k or -k)
 * - User limit status (+l <limit> or -l)
 *
 * @return A string representing the current mode settings of the channel.
 */
std::string Channel::getMode() const
{
	std::string display;
	if (getInvites() == true)
		display = "+i";
	else
		display = "-i";
	if (getRightsTopic() == true)
		display += " +t";
	else
		display += " -t";
	if (getPassword() == "")
		display += " -k";
	else
		display += " +k";
	if (getLimits() == -1)
		display += " -l";
	else
	{
		std::stringstream ss;
		ss << getLimits();  // écrire l'entier dans le flux
		display += " +l " + ss.str();	
	}
	return display;
}

int Channel::getConnectedCount() const
{
	return _connected.size();
}
std::set<const Client*> Channel::getClientsList() const
{
	return _connected;
}
std::set<const Client*> Channel::getOperatorsList() const
{
	return _operators;
}
std::set<const Client*> Channel::getInvitedList() const
{
	return _invited;
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


bool Channel::getInvites() const
{
	return _invites;
}
bool Channel::getRightsTopic() const
{
	return _rightsTopic;
}
int Channel::getLimits() const
{
	return _limits;
}


bool Channel::hasClients() const
{
	return !_connected.empty();
}
bool Channel::hasOperators() const
{
	return !_operators.empty();
}
bool Channel::hasInvites() const
{
	return !_invited.empty();
}
bool Channel::hasPassword() const
{
	return !_password.empty();
}
bool Channel::hasTopic() const
{
	return !_topic.empty();
}
bool Channel::isFull() const
{
	return getConnectedCount() + 1 > _limits && _limits != -1;
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