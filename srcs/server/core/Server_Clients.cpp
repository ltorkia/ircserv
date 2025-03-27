#include "../../../incs/server/Server.hpp"

// === OTHER CLASSES ===
#include "../../../incs/server/Client.hpp"
#include "../../../incs/utils/IrcHelper.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/irc_config.hpp"
#include "../../../incs/config/server_messages.hpp"

using namespace server_messages;

// =========================================================================================

// === CLIENT MANAGER ===

// ========================================= PUBLIC ========================================

/**
 * @brief Returns the list of clients.
 *
 * This function returns the list of clients as a reference.
 *
 * @return std::map<int, Client*>& The list of clients.
 */
std::map<int, Client*>& Server::getClients()
{
	return _clients;
}

/**
 * @brief Get the number of clients connected to the server.
 * 
 * @return int The number of clients.
 */
int Server::getTotalClientCount() const
{
	return _clients.size();
}

/**
 * @brief Get the count of clients based on their authentication status.
 * 
 * @param authenticated If true, count only authenticated clients; 
 *                      if false, count only unauthenticated clients.
 * @return int The number of clients that match the specified authentication status.
 */
int Server::getClientCount(bool authenticated)
{
	int count = 0;
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (authenticated && (it->second)->isAuthenticated())
			count++;
		else if (!authenticated && !(it->second)->isAuthenticated())
			count++;
	}
	return count;
}

/**
 * @brief Retrieves the client ID associated with a given nickname.
 *
 * This function iterates through the list of clients and returns the ID of the client
 * whose nickname matches the provided nickname. If the optional currClient parameter
 * is provided, it will be skipped during the search.
 *
 * @param nickname The nickname of the client to search for.
 * @param currClient Optional parameter to specify a client to be skipped during the search.
 * @return The ID of the client with the matching nickname, or -1 if no match is found.
 */
int Server::getClientByNickname(const std::string &nickname, Client* currClient)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (currClient && currClient == it->second)
			continue;
		if (nickname == (it->second)->getNickname()) 
			return it->first;
	}
	return -1;
}

/**
 * @brief Sends a greeting message to a newly connected client.
 *
 * This function sends a series of welcome messages to the client, including:
 * - A welcome message with the client's username, nickname, and IP address.
 * - Host information.
 * - Server creation time.
 * - Additional server information.
 * - A global user list with the total number of clients, known clients, unknown clients, and channels.
 *
 * @param client A pointer to the Client object representing the newly connected client.
 */
void Server::greetClient(Client* client)
{
	const std::string& nickname = client->getNickname();
	const std::string& usermask = client->getUsermask();

	client->sendMessage(MessageBuilder::ircWelcomeMessage(nickname, usermask), NULL);
	client->sendMessage(MessageBuilder::ircHostInfos(nickname), NULL);
	client->sendMessage(MessageBuilder::ircTimeCreation(nickname, _timeCreationStr), NULL);
	client->sendMessage(MessageBuilder::ircInfos(nickname), NULL);
	client->sendMessage(MessageBuilder::ircMOTDMessage(nickname), NULL);

	int totalClientCount = getTotalClientCount();
	int unknownClientCount = getClientCount(false);
	int knownClientCount = getClientCount(true);
	int channelCount = getChannelCount();

	client->sendMessage(MessageBuilder::ircGlobalUserList(nickname, totalClientCount, knownClientCount, unknownClientCount, channelCount), NULL);
}

/**
 * @brief Broadcasts a message for nickname change to all connected and authenticated clients except those marked for deletion.
 * 
 * This function iterates through the list of connected clients and sends the specified
 * message to each client that is not present in the list of clients to be deleted.
 * 
 * @param message The message to be broadcasted to all connected and authenticated clients.
 */
void Server::broadcastToClients(const std::string &message)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		std::vector<std::map<int, Client*>::iterator>::iterator itDeleted = std::find(_clientsToDelete.begin(), _clientsToDelete.end(), it);
		if (itDeleted == _clientsToDelete.end() && it->second->isAuthenticated())
			it->second->sendMessage(message, NULL);
	}
}

/**
 * @brief Prepares a client to leave the server.
 *
 * This function handles the necessary steps to properly disconnect a client from the server.
 * It ensures that the client leaves all channels they are part of, disconnects the client,
 * and marks the client for deletion.
 *
 * @param it An iterator pointing to the client in the map of clients.
 */
void Server::prepareClientToLeave(std::map<int, Client*>::iterator it, const std::string& reason)
{
	int clientFd = it->first;
	Client* client = it->second;

	client->leaveAllChannels(_channels, reason, leaving_code::QUIT_SERV);
	_disconnectClient(clientFd, reason);
	_clientsToDelete.push_back(it);
}


// ========================================= PRIVATE =======================================

/**
 * @brief Accepts a new client connection to the server.
 *
 * This function handles the acceptance of a new client connection. It performs the following steps:
 * 1. Initializes a sockaddr_in structure to store the client's address.
 * 2. Accepts a new connection and obtains a new socket descriptor for the client.
 * 3. Sets the new client socket to non-blocking mode.
 * 4. Adds the new client to the list of connected clients.
 * 5. Retrieves and stores the client's address.
 * 6. Converts the client's binary address to a readable string format.
 * 7. Sets the client's hostname or defaults to "127.0.0.1" if unavailable.
 * 8. Adds the client's socket descriptor to the set of descriptors monitored for reading.
 * 9. Sets the start of the client's activity.
 * 10. Prompts the client to enter authentication information.
 * 11. Outputs a debug message indicating the client has connected.
 *
 * @note This function uses perror to print error messages if any system call fails.
 */
void Server::_acceptNewClient()
{
	// Structure pour récupérer l'adresse du client qui se connecte
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);

	// Accepter une connexion et obtenir un nouveau descripteur de socket pour ce client
	int newClientFd = accept(_serverSocketFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (newClientFd < 0)
	{
		perror("Failed to accept new client");
		return;
	}

	// Rendre le nouveau socket non-bloquant
	if (fcntl(newClientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		perror("Failed to set client socket to non-blocking");
		_disconnectClient(newClientFd, CONNECTION_FAILED);
		return;
	}
	_addClient(newClientFd);

	Client* client = _clients[newClientFd];

	// Si l'adresse et le port du client ne sont pas récupérables (ex: proxy, VPN...)
	// on assigne des valeurs par défaut pour éviter une déconnexion
	if (getpeername(newClientFd, (struct sockaddr*)&clientAddr, &clientAddrLen) == -1)
	{
		client->setClientIp(server::UNKNOWN_IP);
		client->setClientPort(0);
	}
	else
	{
		// Buffer pour stocker l'adresse IP du client (NI_MAXHOST garantit une taille suffisante)
		char ipAddr[NI_MAXHOST];
	
		// Convertit l'adresse binaire en chaîne lisible
		if (getnameinfo((struct sockaddr*)&clientAddr, clientAddrLen, ipAddr, sizeof(ipAddr), NULL, 0, NI_NUMERICHOST) != 0)
			client->setClientIp(server::UNKNOWN_IP);
		else
			client->setClientIp(ipAddr);
	
		// Stocke le port source du client (0 si non identifiable)
		int clientPort = ntohs(((struct sockaddr_in*)&clientAddr)->sin_port);
		client->setClientPort(clientPort);
	}

	// Ajouter le descripteur du client à l'ensemble des descripteurs surveillés pour l'écriture et la lecture
	FD_SET(newClientFd, &_readFds);

	// Prompt pour saisir les infos d'authentification
	std::string authenticationPrompt = IrcHelper::commandToSend(*client);
	client->sendMessage(MessageBuilder::ircCommandPrompt(authenticationPrompt, "", false), NULL);

	// Log de connexion du client
	std::cout << MessageBuilder::msgClientConnected(client->getClientIp(), client->getClientPort(), newClientFd, "") << std::endl;
}

/**
 * @brief Adds a new client to the server.
 *
 * This function creates a new Client object using the provided client file descriptor
 * and adds it to the server's client map.
 *
 * @param clientFd The file descriptor of the client to be added.
 */
void Server::_addClient(int clientFd)
{
	_clients[clientFd] = new Client(clientFd);
}

/**
 * @brief Checks the activity of all connected clients and handles inactivity.
 *
 * This function iterates through all connected clients and performs the following actions:
 * - If a client has been inactive for more than 4 minutes, it sends a PING message to the client to check the connection.
 * - If a client has been inactive for more than 5 minutes (no PONG or command received), it prepares the client to be disconnected due to a connection timeout.
 */
void Server::_checkActivity()
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		Client* client = it->second;
		time_t idleTime = client->getIdleTime();

		// Au bout de 4 minutes d'inactivité, envoie un PING au client pour vérifier sa connexion
		if (!client->pingSent() && idleTime > server::PING_INTERVAL)
		{
			client->setPingSent(true);
			client->sendMessage(MessageBuilder::ircPing(), NULL);
		}
		// Si le client est inactif depuis 5 minutes (pas de PONG ou de commande reçue), on le déconnecte
		if (idleTime > server::PONG_TIMEOUT)
			prepareClientToLeave(it, CONNECTION_TIMEOUT);
	}
}

/**
 * @brief Disconnects a client from the server.
 *
 * This function removes the client's socket from the set of descriptors to monitor,
 * closes the client's socket, and prints a message indicating the successful disconnection.
 *
 * @param fd The file descriptor of the client to disconnect.
 *
 * @return void
 */
void Server::_disconnectClient(int fd, const std::string& reason)
{
	if (reason == SHUTDOWN_REASON || reason == CONNECTION_TIMEOUT || reason == CONNECTION_FAILED)
		_clients[fd]->sendMessage(MessageBuilder::ircErrorQuitServer(reason), NULL);

	// Retirer le socket du client des descripteurs à surveiller
	FD_CLR(fd, &_readFds);

	// Fermer le socket du client
	if (close(fd) == -1)
	{
		perror("Failed to close client socket");
		return;
	}

	std::string nick = _clients[fd]->isAuthenticated() ? _clients[fd]->getNickname() : "";
	std::cout << MessageBuilder::msgClientDisconnected(_clients[fd]->getClientIp(), _clients[fd]->getClientPort(), fd, nick) << std::endl;
}

/**
 * @brief Deletes a client from the connected clients list.
 *
 * This function removes a client from the map of connected clients and deletes the associated
 * client object. It also closes the client's socket connection.
 *
 * @param it An iterator pointing to the client in the map of connected clients.
 *
 * @return void
 */
void Server::_deleteClient(std::map<int, Client*>::iterator it)
{
	if (it != _clients.end())
	{
		delete it->second; // Supprime l'objet client
		_clients.erase(it->first); // Supprime l'entrée du client dans map
	}
}

/**
 * @brief Deletes clients marked for deletion.
 * 
 * This function iterates through the list of clients that are marked for deletion,
 * deletes each one. After all clients are deleted, the list is cleared.
 */
void Server::_lateClientDeletion()
{
	for (std::vector<std::map<int, Client*>::iterator>::iterator it = _clientsToDelete.begin(); it != _clientsToDelete.end(); ++it)
		_deleteClient(*it);
	_clientsToDelete.clear();
}