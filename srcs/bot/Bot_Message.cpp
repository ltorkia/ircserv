#include "../../incs/bot/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/utils/Utils.hpp"
#include "../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../incs/config/bot_config.hpp"
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/server_messages.hpp"

using namespace bot_config;
using namespace server_messages;

// =========================================================================================

// === READ / SEND MESSAGES ===

// ========================================= PRIVATE =======================================

/**
 * @brief Handles incoming messages from the server and processes them accordingly.
 * 
 * This function reads messages from the server, processes them line by line, and
 * performs actions based on the bot's authentication state and the type of server
 * commands received.
 * 
 * Workflow:
 * 1. Reads data from the server into a buffer.
 * 2. Extracts and cleans individual messages from the buffer.
 * 3. If the bot is not authenticated, processes authentication messages.
 * 4. If the bot is authenticated, handles server commands such as PING, INVITE, JOIN, PRIVMSG, etc.
 * 5. Attempts to authenticate the bot if all authentication information has been sent.
 * 
 * @note The function assumes that messages are delimited by newline characters ('\n').
 *       It also handles cases where messages may include carriage return characters ('\r').
 */
void Bot::_handleMessage()
{
	if (_readFromServer() < 0) 
		return;

	std::string& bufferMessage = _buffer;
	size_t pos;
	while (((pos = bufferMessage.find('\n')) != std::string::npos))
	{
		// On extrait le message jusqu'au \n (non inclus)
		// + on enlève le \r s'il y en a un (cas irssi)
		// + on supprime la commande traitée du buffer
		std::string message = Utils::extractAndCleanMessage(bufferMessage, pos);

		// Debug : affiche le message reçu
		// std::cout << "---> bot received: " << message << std::endl;

		// Les infos d'authentification (PASS, NICK, USER) 
		// ont été envoyées au serveur au lancement du programme.
		// On stocke les réponses du serveur dans un buffer pour vérifier
		// en sortie de boucle que le bot a bien été register
		if (!_isAuthenticated)
		{
			_saveServerResponses(message);
			continue;
		}

		// Si le bot est authentifié, on traite les commandes serveur
		// (PING, INVITE, JOIN, PRIVMSG...)
		_manageServerCommand(message);
	}

	// On authentifie le bot si ce n'est pas déjà fait
	// et que tout s'est bien déroulé côté serveur
	if (!_isAuthenticated)
		_authenticate();
}

/**
 * @brief Reads data from the server into the buffer.
 *
 * This function attempts to read data from the server socket into a local buffer.
 * It appends the read data to the internal buffer of the Bot instance.
 *
 * @return The number of bytes read from the server.
 * @retval -1 If there was an error reading from the server.
 * @throws std::invalid_argument If the connection to the server is closed.
 */
int Bot::_readFromServer()
{
	char currentBuffer[server::BUFFER_SIZE];
	std::memset(currentBuffer, 0, sizeof(currentBuffer));

	int bytesRead = recv(_botFd, currentBuffer, sizeof(currentBuffer) - 1, 0);
	if (bytesRead < 0)
	{
		perror("Failed to read from server");
		return -1;
	}
	if (bytesRead == 0)
		throw std::invalid_argument(MSG_CONNECTION_CLOSED);

	currentBuffer[bytesRead] = '\0';
	_buffer.append(currentBuffer);
	return bytesRead;
}

/**
 * @brief Sends a message from the bot to the IRC server.
 *
 * This function formats the given message according to IRC protocol
 * (adds \r\n and truncates to 512 characters if too long) and sends it
 * through the bot's file descriptor. It also logs the sent message to
 * the standard output.
 *
 * @param message The message to be sent.
 */
void Bot::_sendMessage(const std::string &message) const
{
	if (signalReceived)
		return;
		
	// On formate le message en IRC (ajout du \r\n, si trop long tronqué à 512 caractères)
	std::string formattedMessage = MessageBuilder::ircFormat(message);

	// std::cout << "<--- bot sent: " << formattedMessage << std::endl;

	if (send(_botFd, formattedMessage.c_str(), formattedMessage.length(), MSG_NOSIGNAL) == -1)
		perror("send() failed");
}

/**
 * @brief Announces the bot features to the target.
 *
 * This function checks if the target is either the client nickname or the channel name.
 * If the target is the client nickname, it ensures that the client has not already received
 * the bot prompt. If the client has not received the prompt, it sends a welcome message
 * and marks the client as having received the prompt. If the target is the channel name,
 * it sends a welcome message to the channel.
 *
 * @note The function does nothing if the target is neither the client nickname nor the channel name.
 */
void Bot::_announceBotFeatures()
{
	if (!_clientNickname.empty() && _target != _clientNickname
		&& _target != _channelName)
		return;

	if (!_clientNickname.empty() && _target == _clientNickname)
	{
		if (_activeClients.find(_clientNickname) == _activeClients.end())
			_activeClients[_clientNickname] = false;
			
		bool receivedBotPrompt = _activeClients[_clientNickname];
		if (!receivedBotPrompt)
		{
			_sendMessage(MessageBuilder::botCmdPrivmsg(_clientNickname, MSG_WELCOME_PROMPT));
			_activeClients[_clientNickname] = true;
		}
		return;
	}
	_sendMessage(MessageBuilder::botCmdPrivmsg(_target, MSG_WELCOME_PROMPT));
}