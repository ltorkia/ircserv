/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:43:20 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>				// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <iomanip>				// gestion formatage -> std::setw(), std::setfill()
#include <sstream>				// gestion flux -> std::ostringstream
#include <cstring>				// memset()
#include <fcntl.h>				// fcntl() -> F_SETFL, O_NONBLOCK
#include <netdb.h>				// gestion DNS -> gethostbyname()
#include <cstdio>				// perror()
#include <cerrno>				// codes erreur -> errno
#include <stdexcept>			// exceptions -> std::runtime_error etc...
#include <map>					// container map
#include <vector>				// container vector
#include <set>					// container set

// =========================================================================================

class Server;
class Channel;
class Client
{
	private:
		// =================================================================================
		
		// === VARIABLES ===

		// =================================================================================

		Client();
		Client(const Client& src);
		Client& operator=(const Client& src);

		int _clientSocketFd, _port;														// Descripteur de socket du client + port
		bool _isIrssi, _isIdentified, _authenticated, _rightPassServ;					// Indique si le client est via irssi, si les NICK et USER sont fournis à la connexion, s'il est authentifié, et s'il a le bon mot de passe serveur
		
		std::vector<std::string> _identNicknameCmd, _identUsernameCmd;					// Commande d'identification nickname et username d'Irssi
		std::string _nickname, _username, _realName, _hostname, _clientIp, _usermask;	// Pseudo + nom d'utilisateur + nom réel + nom d'hôte + adresse IP + usermask pour RPL
		
		std::string _bufferMessage;														// Buffer de message
		std::string _awayMessage;														// Message d'absence
		
		time_t _signonTime, _lastActivity;												// Timestamp de connexion et dernier moment actif du client
		bool _isAway, _errorMsgTooLongSent, _pingSent;									// Indique si le client est marqué absent, si une erreur est envoyée car message trop long, et si le serveur attend un PONG

		std::map<std::string, Channel*> _channelsJoined;								// Liste des canaux auxquels le client est connecté

	public:
		// =================================================================================
		// === CLIENT CONSTRUCTOR / DESTRUCTOR === Client.cpp

		Client(int fd);
		~Client();

		// =================================================================================
		
		// === PUBLIC METHODS ===

		// =================================================================================

		// =================================================================================
		// === CLIENT INFOS SETTERS === Client_Attributes.cpp

		// === GENERAL INFOS ===
		void setClientPort(int clientPort);									// Définit le port client
		void setNickname(const std::string &nickname);						// Définit le pseudo du client
		void setUsername(const std::string &username);						// Définit le nom d'utilisateur
		void setRealName(const std::string &realName);						// Définit le nom réel
		void setHostname(const std::string &hostname);						// Définit le nom d'hôte
		void setClientIp(const std::string &clientIp);						// Définit l'adresse IP client
		void setUsermask();													// Définit le usermask du client pour RPL

		// === AUTHENTICATION INFOS ===
		void setIsIrssi(bool status);										// Définit si le client est un Irssi
		void setIdentified(bool status);									// Définit si irssi fournit les nick et user par indent
		void setIdentNickCmd(std::vector<std::string> identCmd);			// Définit la commande d'identification nickname d'Irssi
		void setIdentUsernameCmd(std::vector<std::string> identCmd);		// Définit la commande d'identification username d'Irssi
		void setServPasswordValidity(bool status);							// Définit si le mot de passe est valide
		void authenticate();												// Authentifie le client
		
		// === ACTIVITY INFOS ===
		void setLastActivity();
		void setIsAway(bool status);										// Définit si le client est absent
		void setAwayMessage(const std::string& message);					// Définit le message d'absence du client
		void setErrorMsgTooLongSent(bool status);							// Définit si le message d'erreur d'un input trop long est déjà envoyé
		void setPingSent(bool status);										// Définit si le serveur attend un PONG du client

		// =================================================================================
		// === CLIENT INFOS GETTERS === Client_Attributes.cpp

		// === GENERAL INFOS ===
		int getFd() const;													// Récupère le descripteur de socket du client
		int getClientPort() const;											// Récupère le port client
		const std::string& getNickname() const;								// Récupère le pseudo
		const std::string& getUsername() const;								// Récupère le nom d'utilisateur
		const std::string& getRealName() const;								// Récupère le nom réel
		const std::string& getHostname() const;								// Récupère le nom d'hôte
		const std::string& getClientIp() const;								// Récupère l'adresse IP client
		const std::string& getUsermask() const;								// Récupère le usermask du client pour RPL

		// === AUTHENTICATION INFOS ===
		bool isIrssi() const;												// Vérifie si le client est un Irssi
		bool isIdentified() const;											// Vérifie si irssi fournit les nick et user par ident
		std::vector<std::string> getIdentNickCmd() const;					// Récupère la commande d'identification nickname d'Irssi
		std::vector<std::string> getIdentUsernameCmd() const;				// Récupère la commande d'identification username d'Irssi
		bool gotValidServPassword() const;									// Vérifie si le client a donné le bon mot de passe du serveur
		bool isAuthenticated() const;										// Vérifie si le client est authentifié
		
		// === ACTIVITY INFOS ===
		time_t getSignonTime() const;										// Récupère le timestamp de connexion
		time_t getLastActivity() const;										// Récupère le dernier moment actif du client
		time_t getIdleTime() const;											// Récupère le temps d'inactivité du client
		bool isAway() const;												// Vérifie si le client est absent
		const std::string& getAwayMessage() const;							// Récupère le message d'absence
		bool errorMsgTooLongSent() const;									// Vérifie si le message d'erreur d'un input trop long est déjà envoyé
		bool pingSent() const;												// Dit si le serveur attend un PONG du client		
		
		// === RELATED CHANNELS ===
		std::map<std::string, Channel*>& getChannelsJoined();				// Récupère les canaux auxquels le client est connecté
		bool isInChannel(const std::string& channelName) const;				// Vérifie si le client est membre d'un canal
		bool isOperator(Channel* channel) const;							// Vérifie si le client est un opérateur sur un canal
		bool isInvited(const Channel* channel) const;						// Vérifie si le client est invité sur un canal

		// === BUFFER ===
		std::string& getBufferMessage();									// Récupère le buffer de message

		// =================================================================================
		// === ACTIONS === Client_Actions.cpp

		// === CHANNEL ACTIONS ===
		void joinChannel(const std::string& channelName, const std::string& password, std::map<std::string, Channel*>& channels);									// Rejoint un canal (creer et rejoindre ou rejoindre existant)
		void createChannel(const std::string& channelName, const std::string& password, std::map<std::string, Channel*>& channels);									// Crée un canal
		void addToChannel(Channel* channel, const std::string& password, const std::string& channelName, std::map<std::string, Channel*>& channels);				// Ajoute un client à un canal
		void msgAfterJoin(Channel* channel, const std::string& channelName);																						// Send les bons RPL IRC après un join channel
		bool hasRightPassword(Channel* channel, const std::string& password);																						// Vérifie le mot de passe du canal
		void passwordSetting(Channel* channel, const std::string& password);																						// Définit le mot de passe du canal
		void isKickedFromChannel(Channel *channel, Client* kicker, const std::string& reason);																		// Est exclu d'un canal
		void isInvitedToChannel(Channel *channel, const Client* inviter);
		void leaveChannel(std::map<std::string, Channel*>::iterator it, std::map<std::string, Channel*>& channels, const std::string& reason, int reasonCode);		// Quitte un canal
		void leaveAllChannels(std::map<std::string, Channel*>& channels, const std::string& reason, int reasonCode);												// Quitte tous les canaux
		void deleteChannel(Channel* channel, std::map<std::string, Channel*>& channels);																			// Supprime un canal
				
		// === SEND MESSAGES (TO CLIENTS OR CHANNEL) ===
		void sendMessage(const std::string &message, Client* sender) const;						// Le serveur envoie un message au client
		void sendToAll(Channel* channel, const std::string &message, bool includeSender);		// Envoie un message formaté irc à tous les clients connectés a un channel
};