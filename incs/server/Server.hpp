#pragma once

// === SERVER LIBRARIES ===
#include "../config/server_libs.hpp"

// =========================================================================================

class Client;
class Channel;
class Server
{
	private:
		Server();
		Server(const Server& src);
		Server& operator=(const Server& src);

		// === SERVER INFOS + SOCKETS ===
		std::string _password, _localIp, _timeCreationStr;						// Mot de passe serveur + adresse IP locale + date et heure de création du serveur
		int _serverSocketFd, _port, _maxFd;										// Descripteur du socket du serveur + port + descripteur maximum pour select()
		fd_set _readFds;														// Ensemble des descripteurs surveillés
		
		// === CONTAINERS -> CLIENTS + CHANNELS ===
		std::map<int, Client*> _clients;										// Liste des clients connectés
		std::vector<std::map<int, Client*>::iterator> _clientsToDelete;			// Liste des clients à supprimer (stocke les iterateurs map des clients)
		std::map<std::string, Channel*> _channels;								// Liste des canaux

// =========================================================================================

		// === INIT SERVER === Server_ServerLoop.cpp
		void _init();	
		void _setSignal();														// Paramétrage du signal
		void _setLocalIp();														// Récupère l'adresse IP locale
		void _setServerSocket();												// Paramétrage du socket serveur
		
		// === SERVER LOOP === Server_ServerLoop.cpp
		void _start();															// Démarre le serveur
		
		// === HANDLE MESSAGES === Server_ServerLoop.cpp
		void _handleMessage(std::map<int, Client*>::iterator it);				// Gère la lecture des messages d'un client
		void _processCommand(std::map<int, Client*>::iterator it, 
											std::string message);				// Traite l'entrée du client
		
		// === CLEAN === Server_ServerLoop.cpp
		void _clean();															// Nettoie le serveur avant fermeture

		// === CLIENT MANAGER === Server_ClientManager.cpp
		void _acceptNewClient();												// Accepte une nouvelle connexion client
		void _addClient(int clientFd); 											// Ajoute un client à la liste
		void _checkActivity();													// Vérifie l'activité des clients
		void _disconnectClient(int fd, const std::string& reason); 				// Déconnecte un client du serveur
		void _deleteClient(std::map<int, Client*>::iterator it);				// Supprime un client de la liste
		void _lateClientDeletion();												// Supprime les clients de la liste en différé
	
	public:
		
		// === STATIC SIGNAL VARIABLE + SIGNAL HANDLER === Server.cpp
		static volatile sig_atomic_t signalReceived;							// Indique si un signal a été reçu
		static void signalHandler(int signal);									// Gestionnaire de signaux pour le serveur
		
		// === CONSTUCTOR / DESTRUCTOR === Server.cpp
		Server(const std::string &port, const std::string &password);
		~Server();

		// === LAUNCH SERVER === Server.cpp
		void launch();

		// === SERVER INFOS GETTERS === Server_Infos.cpp
		int getServerSocketFd() const; 																// Récupère le descripteur de socket du serveur
		const std::string& getLocalIP() const; 														// Récupère l'adresse IP locale
		int getPort() const; 																		// Récupère le port du serveur;
		fd_set getReadFds() const; 																	// Récupère l'ensemble des descripteurs surveillés
		int getMaxFd();																				// Récupère le descripteur maximum pour select()
		const std::string& getServerPassword() const; 												// Récupère le mot de passe du serveur
		std::map<std::string, Channel*>& getChannels(); 											// Récupère la liste des canaux
		int getChannelCount() const; 																// Récupère le nombre de canaux

		// === CLIENT MANAGER === Server_ClientManager.cpp
		std::map<int, Client*>& getClients(); 														// Récupère la liste des clients
		int getTotalClientCount() const; 															// Récupère le nombre total de clients
		int getClientCount(bool authenticated); 													// Récupère le nombre de clients authentifiés ou non
		int getClientByNickname(const std::string& nickname, Client* currClient);					// Récupère le client par son pseudo
		void greetClient(Client* client); 															// Accueille un client
		void broadcastToClients(const std::string &message); 										// Envoie un message à tous les clients connectés
		void prepareClientToLeave(std::map<int, Client*>::iterator it, const std::string& reason);	// Prépare un client à quitter le serveur
};