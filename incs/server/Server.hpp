/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 17:48:54 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// === SERVER LIBRARIES ===
#include "server_libs.hpp"

// === FILES STRUCT ===
#include "FileData.hpp"

// =========================================================================================

class Client;
class Channel;
class Server
{
	private:
		Server();
		Server(const Server& src);
		Server& operator=(const Server& src);

		// =================================================================================
		
		// === VARIABLES ===

		// =================================================================================

		// === SERVER INFOS + SOCKETS ===
		std::string _password, _localIp, _timeCreationStr;						// Mot de passe serveur + adresse IP locale + date et heure de création du serveur
		int _serverSocketFd, _port, _maxFd;										// Descripteur du socket du serveur + port + descripteur maximum pour select()
		fd_set _readFds;														// Ensemble des descripteurs surveillés
		
		// === CONTAINERS -> CLIENTS + CHANNELS ===
		std::map<int, Client*> _clients;										// Liste des clients connectés
		std::vector<std::map<int, Client*>::iterator> _clientsToDelete;			// Liste des clients à supprimer (stocke les iterateurs map des clients)
		std::map<std::string, Channel*> _channels;								// Liste des canaux

		// === FILES TO SEND ===
		std::map<std::string, FileData>	_files;									// Liste des fichiers à envoyer avec DCC SEND


		// =================================================================================
		
		// === PRIVATE METHODS ===

		// =================================================================================

		// =================================================================================
		// === SERVER LOOP === Server_ServerLoop.cpp

		// === INIT SERVER ===
		void _init();	
		void _setSignal();														// Paramétrage du signal
		void _setLocalIp();														// Récupère l'adresse IP locale
		void _setServerSocket();												// Paramétrage du socket serveur
		
		// === START LOOP ===
		void _start();															// Démarre le serveur
		
		// === HANDLE MESSAGES ===
		void _handleMessage(std::map<int, Client*>::iterator it);				// Gère la lecture des messages d'un client
		void _processCommand(std::map<int, Client*>::iterator it, 
											std::string message);				// Traite l'entrée du client
		
		// === CLEAN ===
		void _clean();															// Nettoie le serveur avant fermeture

		// =================================================================================
		// === CLIENT MANAGER === Server_Clients.cpp

		void _acceptNewClient();												// Accepte une nouvelle connexion client
		void _addClient(int clientFd);											// Ajoute un client à la liste
		void _checkActivity();													// Vérifie l'activité des clients
		void _disconnectClient(int fd, const std::string& reason);				// Déconnecte un client du serveur
		void _deleteClient(std::map<int, Client*>::iterator it);				// Supprime un client de la liste
		void _lateClientDeletion();												// Supprime les clients de la liste en différé
	
	public:
		// =================================================================================
		
		// === PUBLIC METHODS ===

		// =================================================================================

		// =================================================================================
		// === SERVER SETTINGS === Server.cpp

		// === STATIC SIGNAL VARIABLE + SIGNAL HANDLER ===
		static volatile sig_atomic_t signalReceived;							// Indique si un signal a été reçu
		static void signalHandler(int signal);									// Gestionnaire de signaux pour le serveur
		
		// === CONSTUCTOR / DESTRUCTOR ===
		Server(const std::string &port, const std::string &password);
		~Server();

		// === LAUNCH SERVER ===
		void launch();

		// =================================================================================
		// === SERVER INFOS GETTERS === Server_Infos.cpp

		int getServerSocketFd() const;											// Récupère le descripteur de socket du serveur
		const std::string& getLocalIP() const;									// Récupère l'adresse IP locale
		int getPort() const;													// Récupère le port du serveur;
		int getMaxFd();															// Récupère le descripteur maximum pour select()
		const std::string& getServerPassword() const;							// Récupère le mot de passe du serveur
		std::map<std::string, Channel*>& getChannels();							// Récupère la liste des canaux
		int getChannelCount() const;											// Récupère le nombre de canaux

		// =================================================================================
		// === CLIENT MANAGER === Server_Clients.cpp

		// === GETTERS ===
		std::map<int, Client*>& getClients();															// Récupère la liste des clients
		int getTotalClientCount() const;																// Récupère le nombre total de clients
		int getClientCount(bool authenticated);															// Récupère le nombre de clients authentifiés ou non
		int getClientByNickname(const std::string& nickname, Client* currClient);						// Récupère le client par son pseudo
		
		// === ACTIONS ===
		void greetClient(Client* client);																// Accueille un client
		void broadcastToClients(const std::string &message);											// Envoie un message à tous les clients connectés
		void prepareClientToLeave(std::map<int, Client*>::iterator it, const std::string& reason);		// Prépare un client à quitter le serveur
		
		// === FILES TO SEND ===
		std::map<std::string, FileData>& getFiles();													// Récupère la liste des fichiers à envoyer
		void addFile(const std::string& filename, const std::string& path, const std::string& sender,
																		const std::string& receiver);	// Ajoute un fichier à la liste
		void removeFile(const std::string& filename);													// Supprime un fichier de la liste
};