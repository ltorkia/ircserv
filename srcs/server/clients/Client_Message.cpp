#include "../../../incs/server/Client.hpp"

// === OTHER CLASSES ===
#include "../../../incs/server/Channel.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/irc_config.hpp"

// =========================================================================================

// === MESSAGES ===

// ========================================= PUBLIC ========================================

// === BUFFER ===

std::string& Client::getBufferMessage()
{
	return _bufferMessage;
}


// === SEND MESSAGES ===

/**
 * @brief Sends a message to the client.
 *
 * This function formats the given message using the IRC format and sends it to the client
 * associated with this instance. If the message length exceeds the buffer size, an error
 * message is generated and sent to the sender if provided.
 *
 * @param message The message to be sent.
 * @param sender The client sending the message, used to send error messages if the message is too long.
 */
void Client::sendMessage(const std::string &message, Client* sender) const
{
	// On formate le message en IRC (ajout du \r\n, si trop long tronqué à 512 caractères)
	std::string formattedMessage = MessageBuilder::ircFormat(message);
	if (send(_clientSocketFd, formattedMessage.c_str(), formattedMessage.length(), MSG_NOSIGNAL) == -1)
	{
		perror("send() failed");
		return;
	}

	// Si le message d'origine a été tronqué car trop long, on prévient le sender (cas PRIVMSG).
	// Si ce même message est envoyé dans un channel ou à plusieurs personnes,
	// l'erreur ne sera envoyée qu'une seule fois à l'envoyeur grâce à un booléen qu'on set à true
	if (message.length() > server::BUFFER_SIZE && sender && sender->errorMsgTooLongSent() == false)
	{
		sender->sendMessage(MessageBuilder::ircLineTooLong(sender->getNickname()), NULL);
		sender->setErrorMsgTooLongSent(true);
	}
}

/**
 * @brief Sends a message to all clients in the specified channel.
 *
 * This function sends the given message to all clients in the provided channel.
 * The message is sent from the current client instance.
 *
 * @param channel Pointer to the Channel object where the message will be sent.
 * @param message The message to be sent to all clients in the channel.
 * @param broadcast A boolean flag indicating whether the message should be sent to the client.
 */
void Client::sendToAll(Channel* channel, const std::string &message, bool includeSender)
{
	channel->sendToAll(message, this, includeSender);
}