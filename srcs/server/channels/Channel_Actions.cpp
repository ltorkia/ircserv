#include "../../../incs/server/Channel.hpp"

// === OTHER CLASSES ===
#include "../../../incs/server/Client.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/irc_config.hpp"

// =========================================================================================

// === CLIENT MANAGER ===

/**
 * @brief Adds a client to the channel if they are not already connected.
 * 
 * This function checks if the given client is already connected to the channel.
 * If the client is not connected, it adds the client to the channel's list of
 * connected clients.
 * 
 * @param client A pointer to the Client object to be added to the channel.
 */
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
	if (isInvited(invited))
	{
		inviter->sendMessage(MessageBuilder::ircAlreadyInvitedToChannel(invited->getNickname(), _name), NULL);
		return;
	}
	_invited.insert(invited);
	inviter->sendMessage(MessageBuilder::ircInviting(inviter->getNickname(), invited->getNickname(), _name), NULL);
	invited->sendMessage(MessageBuilder::ircInvitedToChannel(inviter->getNickname(), _name), NULL);
	std::cout << MessageBuilder::msgIsInvitedToChannel(invited->getNickname(), inviter->getNickname(), _name) << std::endl;
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
	if (isOperator(client))
		return;
	_operators.insert(client);
	std::cout << MessageBuilder::msgClientOperatorAdded(client->getNickname(), _name) << std::endl;
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
	if (!isOperator(client))
		return;
	_operators.erase(client);
	std::cout << MessageBuilder::msgClientOperatorRemoved(client->getNickname(), _name) << std::endl;
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
			sendToAll(MessageBuilder::ircClientKickUser(kicker->getUsermask(), _name, client->getNickname(), reason), client, true);
			std::cout << MessageBuilder::msgClientKickedFromChannel(client->getNickname(), kicker->getNickname(), _name, reason) << std::endl;
		}
		if (reasonCode == leaving_code::LEFT)
		{
			sendToAll(MessageBuilder::ircClientPartChannel(client->getUsermask(), _name, reason), client, true);
			client->sendMessage(MessageBuilder::ircCurrentNotInChannel(client->getNickname(), _name), NULL);
			std::cout << MessageBuilder::msgClientLeftChannel(client->getNickname(), _name, reason) << std::endl;
		}
		if (reasonCode == leaving_code::QUIT_SERV)
			sendToAll(MessageBuilder::ircClientQuitServer(client->getUsermask(), reason), client, false);

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
			sender->sendMessage(MessageBuilder::ircInviteOnly(sender->getNickname(), _name), NULL);
			return;
		}
		sender->sendMessage(MessageBuilder::ircCurrentNotInChannel(sender->getNickname(), _name), NULL);
		return;
	}

	for (std::set<const Client*>::iterator it = _connected.begin(); it != _connected.end(); ++it)
	{
		if (includeSender == false && *it == sender)
			continue;
		(*it)->sendMessage(message, sender);
	}
}