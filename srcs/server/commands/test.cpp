#include <sys/stat.h>		// stat()

#include "../../../incs/server/CommandHandler.hpp"

// === OTHER CLASSES ===
#include "../../../incs/utils/Utils.hpp"
#include "../../../incs/utils/IrcHelper.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/server_messages.hpp"

using namespace file;
using namespace server_messages;

// =========================================================================================

// === FILE COMMANDS ===

// ========================================= PRIVATE =======================================

CommandHandler::FileArgs::FileArgs() : self(NULL) {}
CommandHandler::FileArgs::FileArgs(CommandHandler* self, const std::vector<std::string>& args) :
	self(self), args(args) {}

// ==== FILE COMMAND MANAGER ===

void CommandHandler::_handleFile()
{
	std::vector<std::string> entry = Utils::getTokens(*_itInput, splitter::SENTENCE);

	Utils::toUpper(entry.front());
	if (entry.front() != SEND_CMD && entry.front() != GET_CMD)
		throw std::runtime_error(MessageBuilder::ircNeedMoreParams(_client->getNickname(), entry.front()));

	std::vector<std::string> args = Utils::getTokens(entry.back(), splitter::WORD);
	if (args.empty() || args.size() < 2)
		throw std::runtime_error(MessageBuilder::msgFileUsage(entry.front()));

	pthread_t thread;
	FileArgs* fileArgs = new FileArgs(this, args);
	if (entry.front() == SEND_CMD)
		pthread_create(&thread, NULL, _sendFile, fileArgs);
	else
		pthread_create(&thread, NULL, _getFile, fileArgs);
	pthread_detach(thread);
	// pthread_join(thread, NULL);
	// delete fileArgs;
}


// ==== SEND FILE ===

void* CommandHandler::_sendFile(void* args)
{
	FileArgs* data = (FileArgs*)args;
	CommandHandler* self = data->self;
	std::vector<std::string> localArgs = data->args;

	try {
		// On vérifie que le client receveur existe
		std::string receiver = localArgs[0];
		int clientFd = self->_server.getClientByNickname(receiver, self->_client);
		if (IrcHelper::clientExists(clientFd) == false)
		{
			self->_client->sendMessage(MessageBuilder::ircNoSuchNick(self->_client->getNickname(), receiver), NULL);
			delete data;
			return NULL;
		}

		// On paramètre le port chez les deux clients
		int port;
		int serverSocket = self->_openListeningSocket(port);
		self->_client->setClientFilePort(port);
		self->_clients[clientFd]->setClientFilePort(port);

		// Configuration du timeout pour accept
		struct timeval tv;
		tv.tv_sec = 30;  // 30 secondes de timeout
		tv.tv_usec = 0;
		setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

		// On boucle sur tous les fichiers à envoyer
		for (size_t i = 1; i < localArgs.size(); i++)
		{
			// On vérifie que le fichier existe
			std::string path = localArgs[i];
			std::ifstream file(path.c_str(), std::ios::binary);
			if (!file)
			{
				self->_client->sendMessage("Error opening file: " + path, NULL);
				continue;
			}

			// Calcul de la taille du fichier
			file.seekg(0, std::ios::end);
			size_t fileSize = file.tellg();
			file.seekg(0, std::ios::beg);

			// On enregistre le nom du fichier pour notifier le receveur
			std::string filename = self->_getFilename(path);

			// Envoyer un message de confirmation à l'expéditeur et une notification au récepteur
			self->_sendDCCQuery(*self->_client, SEND_CMD, filename, self->_client->getClientIp(), port, fileSize);
			self->_sendDCCQuery(*self->_clients[clientFd], SEND_CMD, filename, self->_client->getClientIp(), port, fileSize);

			self->_client->sendMessage("Waiting for connection to transfer file " + filename, NULL);
			
			// Accepter la connexion avec timeout
			int clientSocket = accept(serverSocket, NULL, NULL);
			if (clientSocket < 0)
			{
				self->_client->sendMessage("Error accepting connection for file: " + filename, NULL);
				file.close();
				continue;
			}

			// Configuration du timeout pour send/recv
			setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
			
			// Transfert du fichier avec confirmation
			if (self->_transferFile(clientSocket, receiver, filename, path, fileSize))
			{
				self->_client->sendMessage("File transfer completed: " + filename, NULL);
				self->_clients[clientFd]->sendMessage("File transfer completed: " + filename, NULL);
			}
			else
			{
				self->_client->sendMessage("File transfer failed: " + filename, NULL);
				self->_clients[clientFd]->sendMessage("File transfer failed: " + filename, NULL);
			}

			// Fermer le socket client après l'envoi du fichier
			close(clientSocket);
		}
		close(serverSocket);
	}
	catch (const std::exception& e)
	{
		if (self && self->_client)
			self->_client->sendMessage("File transfer error: " + std::string(e.what()), NULL);
	}
	
	// // On vérifie que le client receveur existe
	// std::string receiver = localArgs[0];
	// int clientFd = self->_server.getClientByNickname(receiver, self->_client);
	// if (IrcHelper::clientExists(clientFd) == false)
	// 	throw std::runtime_error(MessageBuilder::ircNoSuchNick(self->_client->getNickname(), receiver));

	// // On paramètre le port chez les deux clients
	// int port;
	// int serverSocket = self->_openListeningSocket(port);
	// self->_client->setClientFilePort(port);
	// self->_clients[clientFd]->setClientFilePort(port);

	// // On boucle sur tous les fichiers à envoyer
	// while (localArgs.size() >= 2)
	// {
	// 	// On vérifie que le fichier existe
	// 	std::string path = localArgs[1];
	// 	std::ifstream file(path.c_str(), std::ios::binary);
	// 	if (!file)
	// 	{
	// 		self->_client->sendMessage("Error opening file: " + path, NULL);
	// 		localArgs.erase(localArgs.begin() + 1);
	// 		continue;
	// 	}

	// 	// On enregistre le nom du fichier pour notifier le receveur
	// 	std::string filename = self->_getFilename(path);

	// 	// Envoyer un message de confirmation à l'expéditeur et une notification au récepteur
	// 	self->_sendDCCQuery(*self->_client, SEND_CMD, filename, self->_client->getClientIp(), port);
	// 	self->_sendDCCQuery(*self->_clients[clientFd], SEND_CMD, filename, self->_client->getClientIp(), port);

	// 	int clientSocket = accept(serverSocket, NULL, NULL);
	// 	if (clientSocket < 0)
	// 	{
	// 		self->_client->sendMessage("Error accepting connection", NULL);
	// 		localArgs.erase(localArgs.begin() + 1);
	// 		continue;
	// 	}
	// 	self->_transferFile(clientSocket, receiver, filename, path);

	// 	// Fermer le socket client après l'envoi du fichier
	// 	close(clientSocket);

	// 	// Retirer l'argument traité
	// 	localArgs.erase(localArgs.begin() + 1);
	// }
	// close(serverSocket);
	delete data;
	return NULL;
}


// ==== GET FILE ===

void* CommandHandler::_getFile(void* args)
{
	FileArgs* data = (FileArgs*)args;
	CommandHandler* self = data->self;
	std::vector<std::string> localArgs = data->args;

    try {
        std::string sender = localArgs[0];

        // On vérifie que le client envoyeur existe
        int clientFd = self->_server.getClientByNickname(sender, self->_client);
        if (IrcHelper::clientExists(clientFd) == false)
        {
            self->_client->sendMessage(MessageBuilder::ircNoSuchNick(self->_client->getNickname(), sender), NULL);
            delete data;
            return NULL;
        }
        
        // On vérifie que le port est valide
        int port = self->_client->getClientFilePort();
        if (!IrcHelper::isValidPortRange(port) || self->_clients[clientFd]->getClientFilePort() != port)
        {
            self->_client->sendMessage("Invalid port number", NULL);
            delete data;
            return NULL;
        }

        // Pour chaque fichier à recevoir, établir une nouvelle connexion
        for (size_t i = 1; i < localArgs.size(); i++)
        {
            // On crée un socket pour se connecter au client envoyeur
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0)
            {
                self->_client->sendMessage("Error creating socket", NULL);
                continue;
            }
            
            // Configuration du timeout
            struct timeval tv;
            tv.tv_sec = 30;  // 30 secondes de timeout
            tv.tv_usec = 0;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
            setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
            
            // On se connecte au client envoyeur
            struct sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(port);
            serverAddr.sin_addr.s_addr = inet_addr(self->_clients[clientFd]->getClientIp().c_str());
            
            if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
            {
                self->_client->sendMessage("Error connecting to sender", NULL);
                close(sock);
                continue;
            }

            // On vérifie que le fichier existe
            std::string path = localArgs[1];
            std::ofstream outFile(path.c_str(), std::ios::binary);
            if (!outFile)
            {
                self->_client->sendMessage("Error opening output file", NULL);
                localArgs.erase(localArgs.begin() + 1);
                continue;
            }
            std::string filename = self->_getFilename(path);

			// Calcul de la taille du fichier
			// file.seekg(0, std::ios::end);
			// size_t fileSize = file.tellg();
			// file.seekg(0, std::ios::beg);
            
            self->_client->sendMessage("Requesting file " + filename + " from " + sender, NULL);
            self->_clients[clientFd]->sendMessage("File request received: " + filename + " from " + self->_client->getNickname(), NULL);

            // Réception du fichier
            if (self->_receiveFile(sock, outFile))
            {
                self->_client->sendMessage("File received: " + filename, NULL);
                self->_clients[clientFd]->sendMessage("File sent successfully: " + filename, NULL);
            }
            else
            {
                self->_client->sendMessage("Failed to receive file: " + filename, NULL);
                self->_clients[clientFd]->sendMessage("Failed to send file: " + filename, NULL);
            }
            
            close(sock);
        }
    }
    catch (const std::exception& e)
    {
        if (self && self->_client)
            self->_client->sendMessage("File reception error: " + std::string(e.what()), NULL);
    }

	// std::string sender = localArgs[0];

	// // On vérifie que le client envoyeur existe
	// int clientFd = self->_server.getClientByNickname(sender, self->_client);
	// if (IrcHelper::clientExists(clientFd) == false)
	// 	throw std::runtime_error(MessageBuilder::ircNoSuchNick(self->_client->getNickname(), sender));
	
	// // On vérifie que le port est valide
	// int port = self->_client->getClientFilePort();
	// if (!IrcHelper::isValidPortRange(port) || self->_clients[clientFd]->getClientFilePort() != port)
	// 	throw std::runtime_error("Invalid port number");

	// // On crée un socket pour se connecter au client envoyeur
	// int sock = socket(AF_INET, SOCK_STREAM, 0);
	// if (sock < 0)
	// 	throw std::runtime_error("Error creating socket");
	
	// // On se connecte au client envoyeur
	// struct sockaddr_in serverAddr;
	// serverAddr.sin_family = AF_INET;
	// serverAddr.sin_port = htons(port);
	// serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	// if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	// 	throw std::runtime_error("Error connecting to sender");

	// // On boucle sur tous les fichiers à envoyer
	// while (localArgs.size() >= 2)
	// {	
	// 	// On vérifie que le fichier existe
	// 	std::string path = localArgs[1];
	// 	std::ofstream outFile(path.c_str(), std::ios::binary);
	// 	if (!outFile)
	// 	{
	// 		self->_client->sendMessage("Error opening output file", NULL);
	// 		localArgs.erase(localArgs.begin() + 1);
	// 		continue;
	// 	}

	// 	// On enregistre le nom du fichier pour notifier l'envoyeur
	// 	std::string filename = self->_getFilename(path);

	// 	self->_clients[clientFd]->sendMessage("receiving file " + filename + " from " + self->_client->getNickname(), NULL);

	// 	// On reçoit le fichier depuis le client envoyeur
	// 	char buffer[4096];
	// 	ssize_t bytesRead;
	// 	while ((bytesRead = recv(sock, buffer, 4096, 0)) > 0)
	// 		outFile.write(buffer, bytesRead);
	// 	if (bytesRead < 0)
	// 		perror("Error receiving file");
	// 	outFile.close();

	// 	self->_clients[clientFd]->sendMessage("received file " + filename + " from " + self->_client->getNickname(), NULL);

	// 	// Retirer l'argument traité
	// 	localArgs.erase(localArgs.begin() + 1);
	// }
	
	// close(sock);
	delete data;
	return NULL;
}

int CommandHandler::_openListeningSocket(int &port) const
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		throw std::runtime_error("Error creating socket");
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = 0;
	
	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Error binding socket");
	
	socklen_t len = sizeof(addr);
	if (getsockname(sock, (struct sockaddr*)&addr, &len) < 0)
		throw std::runtime_error("Error getting socket name");
	
	port = ntohs(addr.sin_port);
	if (!IrcHelper::isValidPortRange(port))
		throw std::runtime_error("Invalid port number");
	listen(sock, 1);
	return sock;
}

bool CommandHandler::_transferFile(int clientSocket, const std::string& receiver, const std::string& filename, const std::string& path, size_t fileSize) const
{
	(void)fileSize;
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file)
	{
		_client->sendMessage("Error opening file: " + path, NULL);
		return false;
	}
	
	_client->sendMessage("sending file " + filename + " for " + receiver, NULL);

	char buffer[4096];
	while (file.read(buffer, 4096) || file.gcount() > 0)
	{
		if (send(clientSocket, buffer, file.gcount(), 0) < 0)
		{
			perror("Error sending file");
			file.close();
			return false;
		}
	}
	file.close();

	_client->sendMessage("sent file " + filename + " for " + receiver, NULL);
	return true;
}

bool CommandHandler::_receiveFile(int clientSocket, std::ofstream& outFile)
{
    // On reçoit le fichier depuis le client envoyeur
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, 4096, 0)) > 0)
        outFile.write(buffer, bytesRead);
    if (bytesRead < 0)
    {
        perror("Error receiving file");
        outFile.close();
        return false;
    }
    outFile.close();
    return true;
}

/**
 * @brief Extracts the filename from a given file path.
 * 
 * This function takes a file path as input and returns the filename
 * by finding the last occurrence of the '/' character and extracting
 * the substring that follows it.
 * 
 * @param path The full file path as a string.
 * @return A string containing the filename extracted from the path.
 */
std::string CommandHandler::_getFilename(std::string path) const
{		
	size_t pos = path.find_last_of('/');
	return path.substr(pos + 1);
}

std::string CommandHandler::_convertIpToInt(const std::string& ip)
{
	struct in_addr addr;
	if (inet_pton(AF_INET, ip.c_str(), &addr) == 1)
		return Utils::intToString(ntohl(reinterpret_cast<uint32_t&>(addr.s_addr)));
	std::cerr << "Invalid IP address" << std::endl;
	return "0";
}

std::string CommandHandler::_convertIntToIp(uint32_t ipInt)
{
	struct in_addr addr;
	reinterpret_cast<uint32_t&>(addr.s_addr) = htonl(ipInt);
	char ipStr[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &addr, ipStr, INET_ADDRSTRLEN))
		return std::string(ipStr);
	return "Invalid IP";
}

void CommandHandler::_sendDCCQuery(const Client& client, const std::string& subCommand, const std::string& filename, const std::string& ip, int port, size_t fileSize)
{
	(void)fileSize;
	std::string ipAsInt = _convertIpToInt(ip);
	std::string query = std::string("\x01") + "DCC " + subCommand + " " + filename + " " + ipAsInt + " " + Utils::intToString(port) + std::string("\x01");
	client.sendMessage(query, NULL);
}