#include "../../../incs/server/Server.hpp"

// === OTHER CLASSES ===
#include "../../../incs/server/Client.hpp"
#include "../../../incs/server/CommandHandler.hpp"
#include "../../../incs/utils/Utils.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/irc_config.hpp"
#include "../../../incs/config/colors.hpp"
#include "../../../incs/config/server_messages.hpp"

using namespace server_messages;
using namespace colors;

// =========================================================================================

// === INIT SERVER ===

/**
 * @brief Initializes the server by setting up signals, local IP, server socket,
 *        writing environment file, and displaying welcome message.
 * 
 * This function performs the following steps:
 * - Sets up signal handling.
 * - Determines and sets the local IP address.
 * - Configures and sets up the server socket.
 * - Generates the server creation time string.
 * - Writes the environment file with the local IP and port.
 * - Displays a welcome message with the local IP, port, and password.
 */
void Server::_init()
{
	_setSignal();
	_setLocalIp();
	_setServerSocket();

	_timeCreationStr = MessageBuilder::msgTimeServerCreation();
	Utils::writeEnvFile(_localIp, _port, _password);
	MessageBuilder::displayWelcome(_localIp, _port, _password);
}

/**
 * @brief Sets the signal handler for SIGINT and SIGTSTP signals.
 *
 * This function sets the signal handler for SIGINT and SIGTSTP signals to the `signalHandler` function.
 * The signal handler function is responsible for handling the termination of the server gracefully.
 *
 * @return void
 *
 * @throws std::runtime_error If an error occurs while setting the signal handler.
 *
 * @see signalHandler
 */
void Server::_setSignal()
{
	struct sigaction sa;
	sa.sa_handler = &Server::signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTSTP, &sa, NULL) == -1)
		throw std::runtime_error(ERR_SET_SIGNAL);
}

/**
 * @brief Configures the local IP address for the server to allow clients to connect from other machines.
 * 
 * This function retrieves the list of available network interfaces and sets the server's local IP address
 * to the first valid IPv4 address found, excluding the loopback address (127.0.0.1). If no valid address
 * is found, it defaults to the loopback address. If no network interfaces are available, it throws a runtime error.
 * 
 * @throws std::runtime_error If no valid network interface is found.
 */
void Server::_setLocalIp() // Configure l'adresse réseau pour permettre aux clients de se connecter depuis d'autres machines
{
	// Déclare un pointeur vers une structure contenant les adresses des interfaces réseau
	struct ifaddrs *networkInterfaces, *currInterface;

	// Récupère la liste des interfaces réseau disponibles
	// En cas d'échec, on utilise l'adresse locale par défaut (127.0.0.1)
	if (getifaddrs(&networkInterfaces) == -1)
	{
		_localIp = server::LOCALHOST;
		return;
	}

	// Parcourir la liste des interfaces réseau obtenues
	for (currInterface = networkInterfaces; currInterface; currInterface = currInterface->ifa_next) {

		// Vérifie si l'interface possède une adresse assignée et si c'est une adresse IPv4 (AF_INET)
		if (!currInterface->ifa_addr || currInterface->ifa_addr->sa_family != AF_INET)
			continue;

		// Convertit l'adresse IPv4 en chaîne de caractères lisible
		struct sockaddr_in* addr = (struct sockaddr_in*)currInterface->ifa_addr;
		char ipAddr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &addr->sin_addr, ipAddr, sizeof(ipAddr));

		// Ignore 127.0.0.1 pour ne garder que les adresses réseau réelles
		if (std::string(ipAddr) != server::LOCALHOST)
		{
			_localIp = ipAddr; // Stocke la première adresse IP valide trouvée
			break;
		}
	}

	// Libère la mémoire allouée pour la liste des interfaces réseau
	freeifaddrs(networkInterfaces);

	if (_localIp.empty())
		throw std::runtime_error(ERR_NO_NETWORK);
}

/**
 * @brief Sets up the server socket, binds it to an address and port, and listens for incoming connections.
 *
 * This function creates a server socket, sets it to non-blocking mode, and binds it to a specific address and port.
 * It also listens for incoming connections and initializes the necessary variables for the server to function properly.
 *
 * @return void
 *
 * @throws std::runtime_error If any error occurs during the setup of the server socket.
 */
void Server::_setServerSocket()
{
	// Création du socket serveur (socket TCP)
	_serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocketFd < 0)
		throw std::runtime_error(ERR_SOCKET_CREATION);

	// Réutilisation de l'adresse et du port :
	// Après un arrêt brutal du serveur, si ce dernier n'a pas libéré immédiatement le port, 
	// SO_REUSEADDR permet de réutiliser le port pour se connecter sans attendre le délai habituel.
	int opt = 1;
	if (setsockopt(_serverSocketFd, SOCK_STREAM, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error(ERR_SET_SOCKET);

	// Rendre le socket non-bloquant avec O_NONBLOCK :
	// Le serveur doit être capable de gérer plusieurs connexions simultanées sans forking (interdit ici).
	// Il doit donc être non-bloquant (ne pas se bloquer tant qu'il n'a pas de nouvelles connexions, traité de message etc...)
	// On pourra donc traiter tous nos clients en continu dans une boucle sans interruption -> voir dans start()
	if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error(ERR_SET_SERVER_NON_BLOCKING);

	// Définition de l'adresse du serveur
	// -> adresse IPv4 + port du serveur
	struct sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));  // Initialisation à 0 de la structure
	serverAddr.sin_family = AF_INET; // Type d'adresse (IPv4)
	serverAddr.sin_addr.s_addr = INADDR_ANY; // Accepte toutes les adresses IP locales
	serverAddr.sin_port = htons(_port); // Port sur lequel écouter (converti en format réseau)

	// Associer l'adresse définie au socket
	if (bind(_serverSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
		throw std::runtime_error(ERR_BIND_SOCKET);

	// Ecouter les connexions entrantes
	// -> FD_SETSIZE spécifie le nombre max de connexions simultanées autorisées par select() (ici 1024)
	if (listen(_serverSocketFd, FD_SETSIZE) < 0)
		throw std::runtime_error(ERR_LISTEN_SOCKET);

	// _readFds = ensemble de descripteurs à surveiller pour les connexions/déconnexions, lecture/envoi de messages...
	// Sera mis à jour via une copie temporaire dans la boucle infinie de start()
	// FD_ZERO initialise _readFds à zéro pour s'assurer qu'il est vide avant d'y ajouter de nouveaux descripteurs
	FD_ZERO(&_readFds);

	// Ajout du socket du serveur (_serverSocketFd) à l'ensemble de descripteurs (_readFds)
	// pour écouter les connexions entrantes
	// NB: Nous n'ajoutons pas le serveur à l'ensemble de descripteurs pour l'écriture (_writeFds)
	FD_SET(_serverSocketFd, &_readFds); // Ajouter le socket serveur à l'ensemble

	// Le serveur peut maintenant accepter les connexions entrantes via select()
}


// === SERVER LOOP ===

/**
 * @brief Starts the IRC server.
 *
 * This function initializes the server socket, listens for incoming client connections,
 * and handles client messages using the select() system call. It also manages client deletion
 * and handles exceptions.
 *
 * @return void
 *
 * @throws std::exception If an error occurs during server initialization or execution.
 */
void Server::_start()
{
	// Boucle infinie pour écouter les connexions des clients tant que le serveur n'est pas interrompu
	while (1)
	{
		if (signalReceived)
			break;

		// Copie temporaire de l'ensemble des descripteurs à surveiller pour la lecture (_readFds) pour select()
		// qu'on réactualise à chaque itération de la boucle pour prendre en compte
		// les nouvelles connexions / déconnexions
		fd_set readFds = _readFds;

		// Récupérer le descripteur maximum pour select()
		// -> Si pas de client, ce sera le descripteur du serveur
		// -> Sinon, ce sera le descripteur du client avec le plus grand descripteur
		// 	qui se trouve a la fin de la map car celle-ci est automatiquement triée par ordre croissant
		_maxFd = getMaxFd();

		// Délai pour la fonction select: intervalle de 500 ms pour le retour de fonction
		struct timeval timeout = {0, 500000};

		// Attendre que l'un des descripteurs soit prêt pour la lecture ou l'écriture
		if (select(_maxFd + 1, &readFds, NULL, NULL, &timeout) < 0 && errno != EINTR)
			throw std::runtime_error(ERR_SELECT_SOCKET);

		// Envoi d'un PING à tous les clients inactifs pour vérifier leur connexion
		_checkActivity();

		// On parcourt tous les fds actifs.
		// Si le fd est le fd du serveur : autre fd tente de se connecter,
		// on accepte la nouvelle connexion et on cree un nouveau client.
		// Sinon, le fd est deja client, donc on traite son message.
		for (int fd = 0; fd <= _maxFd; fd++)
		{
			if (signalReceived)
				break;
			if (FD_ISSET(fd, &readFds))
			{
				if (fd == _serverSocketFd)
					_acceptNewClient();
				else
				{
					// On retrouve l'iterateur du client correspondant au fd dans la map _clients
					std::map<int, Client*>::iterator it = _clients.find(fd);
					_handleMessage(it);
				}

			}
		}

		// Supprimer les clients en attente de suppression
		// (les supprimer au fur et à mesure dans la boucle ci-dessus impliquerait
		// de modifier le conteneur pendant l'itération, ce qui causerait un comportement indéfini)
		_lateClientDeletion();
	}
}


// === HANDLE MESSAGES ===

/**
 * @brief Handles incoming messages from a specific client.
 *
 * This function receives messages from a client, processes them, and performs
 * necessary actions based on the received commands. It also manages client deletion
 * and handles exceptions.
 *
 * @param it An iterator pointing to the client in the map of connected clients.
 *
 * @return void
 *
 * @throws std::exception If an error occurs while processing the client's message.
 */
void Server::_handleMessage(std::map<int, Client*>::iterator it)
{
	int clientFd = it->first;
	Client* client = it->second;
	client->setLastActivity();

	char currentBuffer[server::BUFFER_SIZE];
	std::memset(currentBuffer, 0, sizeof(currentBuffer));
	ssize_t bytesRead = recv(clientFd, currentBuffer, sizeof(currentBuffer) - 1, 0);

	if (bytesRead < 0)
	{
		// Erreur de lecture
		perror("Failed to read from client");
		return;
	}
	if (bytesRead == 0)
	{
		// Client déconnecté proprement
		prepareClientToLeave(it, CLIENT_CLOSED_CONNECTION);
		return;
	}
	currentBuffer[bytesRead] = '\0';

	// On récupère le message stocké dans le buffer du client
	std::string& bufferMessage = client->getBufferMessage();

	// Ajoute les nouvelles données reçues au buffer existant
	bufferMessage.append(currentBuffer);

	// On parcourt les messages tant qu'il y a un \n
	size_t pos;
	while (((pos = bufferMessage.find('\n')) != std::string::npos))
	{
		// On extrait le message jusqu'au \n (non inclus)
		// + on enlève le \r s'il y en a un (cas irssi)
		// + on supprime la commande traitée du buffer
		std::string message = Utils::extractAndCleanMessage(bufferMessage, pos);

		// Debug : affiche le message reçu
		// std::cout << "---> " << message << std::endl;

		// On traite le message extrait,
		// le reste sera traité à la prochaine itération
		_processCommand(it, message);
	}

	// S'il reste un message dans le buffer c'est because CTRL+D
	// On l'a déjà stocké dans le buffer, ça sera traité la fois suivante
	if (!bufferMessage.empty() && bufferMessage.size() < server::BUFFER_SIZE - 1)
		client->sendMessage("^D", NULL);
}

/**
 * @brief Processes the input message from a client.
 *
 * This function takes an iterator to a map of clients and a message string,
 * and processes the input message from the client associated with the iterator.
 * It creates a CommandHandler object to manage the command contained in the message.
 * If an exception is thrown during command management, the exception message is sent
 * back to the client.
 *
 * @param it Iterator to a map of clients, where the key is an integer and the value is a pointer to a Client object.
 * @param message The input message from the client to be processed.
 */
void Server::_processCommand(std::map<int, Client*>::iterator it, std::string message)
{
	Client* client = it->second;
	if (client->errorMsgTooLongSent() == true)
		client->setErrorMsgTooLongSent(false);

	try
	{
		CommandHandler handler(*this, it);
		handler.manageCommand(message);
	}
	catch (const std::exception &e)
	{
		client->sendMessage(e.what(), NULL);
	}
}


// === CLEAN ===

/**
 * @brief cleans the IRC server and closes all connections.
 *
 * This function closes all client objects and connections, frees memory,
 * and closes the server socket. It also ensures that all open file descriptors
 * are properly closed.
 * Called in server destructor and signal handler.
 * 
 * @return void
 */
void Server::_clean()
{
	// Fermer toutes connexions clients + objets clients + channels
	while (!_clients.empty())
	{
		int clientFd = _clients.begin()->first;
		Client* client = _clients.begin()->second;

		client->leaveAllChannels(_channels, SHUTDOWN_REASON, leaving_code::QUIT_SERV);
		_disconnectClient(clientFd, SHUTDOWN_REASON);
		_deleteClient(_clients.begin());
	}

	// Fermer le socket du serveur
	if (close(_serverSocketFd) == -1)
	{
		perror("Failed to close server socket");
		return;
	}

	FD_ZERO(&_readFds);
	std::cout << MessageBuilder::msgBuilder(COLOR_SUCCESS, SERVER_SHUT_DOWN, eol::UNIX) << std::endl;
}