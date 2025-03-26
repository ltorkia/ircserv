#include "../../incs/classes/core/Channel.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/core/Client.hpp"
#include "../../incs/classes/utils/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"

// =========================================================================================
/**************************************** PUBLIC ****************************************/

// === CONSTUCTORS / DESTRUCTORS ===

// --- PUBLIC
Channel::Channel(const std::string &name, const std::string& password) : _name(name), _password(password),
	_topic(""), _channelTimestamp(time(0)), _invites(false), _rightsTopic(false), _limits(-1) {}
Channel::~Channel() {}

// --- PRIVATE
Channel::Channel() {}
Channel::Channel(const Channel& src) {(void) src;}
Channel & Channel::operator=(const Channel& src) {(void) src; return *this;}


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
		setter->sendMessage(MessageHandler::ircTopic(setter->getNickname(), _name, _topic), NULL);
		setter->sendMessage(MessageHandler::ircTopicWhoTime(setter->getNickname(), _topicSetterMask, _name, _topicTimestamp), NULL);
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


// === SETTERS / UPDATE CLIENTS LISTS ===

void Channel::addClient(Client* client)
{
	if (!isConnected(client))
		_connected.insert(client);
}

/**
 * @brief Adds a client to the invited list of the channel.
 *
 * This function checks if the client is already invited to the channel.
 * If not, it adds the client to the invited list and sends appropriate
 * messages to both the inviter and the invited client. If the client is
 * already invited, it sends a message to the inviter indicating that the
 * client is already invited.
 *
 * @param invited Pointer to the client being invited.
 * @param inviter Pointer to the client who is sending the invitation.
 */
void Channel::addClientToInvitedList(const Client* invited, const Client* inviter)
{
	if (!isInvited(invited))
	{
		_invited.insert(invited);
		inviter->sendMessage(MessageHandler::ircInviting(inviter->getNickname(), invited->getNickname(), _name), NULL);
		invited->sendMessage(MessageHandler::ircInvitedToChannel(inviter->getNickname(), _name), NULL);
		std::cout << MessageHandler::msgIsInvitedToChannel(invited->getNickname(), inviter->getNickname(), _name) << std::endl;
	} 
	else
		inviter->sendMessage(MessageHandler::ircAlreadyInvitedToChannel(invited->getNickname(), _name), NULL);
}

/**
 * @brief Adds a client as an operator to the channel.
 *
 * This function checks if the given client is already an operator of the channel.
 * If the client is not an operator, it adds the client to the set of operators.
 * It also prints a message to the standard output indicating that the client
 * has been made an operator of the channel.
 *
 * @param client A pointer to the Client object to be added as an operator.
 */
void Channel::addOperator(Client* client)
{
	if (!isOperator(client))
	{
		_operators.insert(client);
		std::cout << MessageHandler::msgClientOperatorAdded(client->getNickname(), _name) << std::endl;
		return;
	}
	// message is already operator
}

/**
 * @brief Removes a client from the list of operators of the channel.
 *
 * This function checks if the given client is an operator of the channel.
 * If the client is an operator, it removes the client from the list of operators.
 * It also prints a message indicating that the client no longer operates on the channel.
 *
 * @param client A pointer to the Client object to be removed from the list of operators.
 */
void Channel::removeOperator(Client* client)
{
	if (isOperator(client))
	{
		_operators.erase(client);
		std::cout << MessageHandler::msgClientOperatorRemoved(client->getNickname(), _name) << std::endl;
	}
}

/**
 * @brief Removes a client from the channel.
 *
 * This function removes the specified client from the channel. If the client is connected,
 * they are removed from the list of connected clients and operators. The channel is also
 * removed from the client's list of joined channels. Depending on whether a kicker name is
 * provided, a different message is sent to all clients in the channel.
 *
 * @param client Pointer to the client to be removed.
 * @param kickerName Name of the client who kicked the target client. If empty, the client left voluntarily.
 * @param reason Reason for the client leaving or being kicked.
 */
void Channel::removeClient(Client* client, const Client* kicker, const std::string& reason, int reasonCode)
{
	if (isConnected(client))
	{

		if (kicker && reasonCode == leaving_code::KICKED)
		{
			sendToAll(MessageHandler::ircClientKickUser(kicker->getUsermask(), _name, client->getNickname(), reason), client, true);
			std::cout << MessageHandler::msgClientKickedFromChannel(client->getNickname(), kicker->getNickname(), _name, reason) << std::endl;
		}
		if (reasonCode == leaving_code::LEFT)
		{
			sendToAll(MessageHandler::ircClientPartChannel(client->getUsermask(), _name, reason), client, true);
			client->sendMessage(MessageHandler::ircCurrentNotInChannel(client->getNickname(), _name), NULL);
			std::cout << MessageHandler::msgClientLeftChannel(client->getNickname(), _name, reason) << std::endl;
		}
		if (reasonCode == leaving_code::QUIT_SERV)
			sendToAll(MessageHandler::ircClientQuitServer(client->getUsermask(), reason), client, false);

		// On supprime le client des clients connectes au canal
		_connected.erase(client);

		// On l'enleve des operateurs s'il est operateur
		removeOperator(client);

		// On retire le canal des canaux du clients
		client->getChannelsJoined().erase(_name);
	}
}


// === MESSAGES ===

/**
 * @brief Sends a message to all clients in the channel.
 *
 * This function sends the specified message to all clients currently connected
 * to the channel. If the sender is not in the channel, an appropriate error
 * message is sent back to the sender. If the channel is invite-only and the
 * sender is not invited, an invite-only error message is sent back to the sender.
 *
 * @param message The message to be sent to all clients in the channel.
 * @param sender The client sending the message.
 * @param includeSender A boolean flag indicating whether the sender should also
 *                      receive the message. If false, the sender will not receive
 *                      the message.
 */
void Channel::sendToAll(const std::string &message, Client* sender, bool includeSender)
{
	if (!sender->isInChannel(_name))
	{
		if (_invites && isInvited(sender) == false)
		{
			sender->sendMessage(MessageHandler::ircInviteOnly(sender->getNickname(), _name), NULL);
			return;
		}
		sender->sendMessage(MessageHandler::ircCurrentNotInChannel(sender->getNickname(), _name), NULL);
		return;
	}

	for (std::set<const Client*>::iterator it = _connected.begin(); it != _connected.end(); ++it)
	{
		if (includeSender == false && *it == sender)
			continue;
		(*it)->sendMessage(message, sender);
	}
}