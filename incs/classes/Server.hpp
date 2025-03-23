#pragma once

// === SERVER LIBRARIES ===
#include "../config/server_libs.hpp"

// === NAMESPACES ===
#include "../config/irc_config.hpp"
#include "../config/colors.hpp"
#include "../config/server_messages.hpp"

// === CLASSES ===
#include "MessageHandler.hpp"
#include "Utils.hpp"
#include "IrcHelper.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Bot.hpp"
#include "CommandHandler.hpp"
#include "CommandHandler_File.hpp"

// =========================================================================================

class Server {

	private:
		Server();
		Server(const Server& src);
		Server& operator=(const Server& src);

		// === SERVER INFOS ===
		int _port;																// Port du serveur
		std::string _password;													// Mot de passe du serveur
		std::string _localIp;													// Adresse IP locale
		std::string _timeCreationStr;											// Date et heure de création du serveur

		// === SOCKETS ===
		int _serverSocketFd;													// Descripteur du socket du serveur
		int _maxFd;																// Descripteur maximum pour select()
		fd_set _readFds;														// Ensemble des descripteurs surveillés
		
		// === CONTAINERS -> CLIENTS + CHANNELS ===
		std::map<int, Client*> _clients;										// Liste des clients connectés
		std::vector<std::map<int, Client*>::iterator> _clientsToDelete;			// Liste des clients à supprimer (stocke les iterateurs map des clients)
		std::map<std::string, Channel*> _channels;								// Liste des canaux

		// === FILES TO SEND ===
		std::map<std::string, File>	_files; 									// Liste des fichiers à envoyer

		// === BOT INSTANCE ===
		Bot* _bot;																// Instance du bot

		// === INIT / CLEAN ===
		void _setSignal();														// Paramétrage du signal
		void _setLocalIp();														// Récupère l'adresse IP locale
		void _setServerSocket();												// Paramétrage du socket serveur

		void _init();															// Initialise le serveur
		void _checkActivity();													// Vérifie l'activité des clients
		void _start();															// Démarre le serveur
		void _clean();															// Nettoie le serveur avant fermeture
		
		// === MESSAGES / COMMANDS ===
		void _handleMessage(std::map<int, Client*>::iterator it);				// Gère la lecture des messages d'un client
		void _processInput(std::map<int, Client*>::iterator it, 
											std::string message);				// Traite l'entrée du client
		
		// === UPDATE CLIENTS ===
		void _acceptNewClient();												// Accepte une nouvelle connexion client
		void _addClient(int clientFd); 											// Ajoute un client à la liste
		void _initBot(int botFd); 												// Ajoute un bot au serveur
		void _disconnectClient(int fd, const std::string& reason); 				// Déconnecte un client du serveur
		void _deleteClient(std::map<int, Client*>::iterator it);				// Supprime un client de la liste
		void _lateClientDeletion();												// Supprime les clients de la liste en différé
	
	public:
		
		// === SIGNAL ===
		static volatile sig_atomic_t signalReceived;							// Indique si un signal a été reçu
		static void signalHandler(int signal);									// Gestionnaire de signaux pour le serveur
		sig_atomic_t isSignalReceived() const; 									// Vérifie si un signal a été reçu
		
		Server(const std::string &port, const std::string &password);
		~Server();

		void launch();
		void cleanExit(); 																			// Fermeture propre du serveur

		// === SERVER INFOS ===
		int getServerSocketFd() const; 																// Récupère le descripteur de socket du serveur
		const std::string& getLocalIP() const; 														// Récupère l'adresse IP locale
		int getPort() const; 																		// Récupère le port du serveur;
		fd_set getReadFds() const; 																	// Récupère l'ensemble des descripteurs surveillés
		int getMaxFd();																				// Récupère le descripteur maximum pour select()
		const std::string& getServerPassword() const; 												// Récupère le mot de passe du serveur
		Bot* getBot(); 																				// Récupère l'instance du bot

		// === CLIENTS ===
		std::map<int, Client*>& getClients(); 														// Récupère la liste des clients
		int getTotalClientCount() const; 															// Récupère le nombre total de clients
		int getClientCount(bool authenticated); 													// Récupère le nombre de clients authentifiés ou non
		int getClientByNickname(const std::string& nickname, Client* currClient);					// Récupère le client par son pseudo
		void greetClient(Client* client); 															// Accueille un client
		void prepareClientToLeave(std::map<int, Client*>::iterator it, const std::string& reason);	// Prépare un client à quitter le serveur

		// === CHANNELS ===
		std::map<std::string, Channel*>& getChannels(); 											// Récupère la liste des canaux
		int getChannelCount() const; 																// Récupère le nombre de canaux
		void broadcastToClients(const std::string &message); 										// Envoie un message à tous les clients connectés

		// === BONUS ===
		std::map<std::string, File>& getFiles(); 													// Récupère la liste des fichiers à envoyer
};