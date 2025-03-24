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
		
		Client();
		Client(const Client& src);
		Client& operator=(const Client& src);

		int _clientSocketFd;								// Descripteur de socket du client
		bool _authenticated;								// Indique si le client est authentifié
		bool _rightPassServ;								// Indique si le client a le bon password du serveur
		
		time_t _signonTime;									// Timestamp de connexion
		time_t _lastActivity;								// Dernier moment actif du client

		bool _isIrssi;										// Indique si le client est un Irssi
		bool _isIdentified;									// Indique si irssi fournit les nick et user automatiquement

		std::vector<std::string> _identNicknameCmd;			// Commande d'identification nickname d'Irssi
		std::vector<std::string> _identUsernameCmd;			// Commande d'identification username d'Irssi

		std::string _bufferMessage;							// Buffer de message

		std::string _nickname;								// Pseudo du client
		std::string _username;								// Nom d'utilisateur
        std::string _realName;								// Nom réel
		std::string _hostname;								// Nom d'hôte
		std::string _clientIp;								// Adresse IP client
		std::string _usermask;								// Usermask du client pour RPL
		int _port;											// Port client
		
		bool _isAway;										// Indique si le client est indiqué absent
		std::string _awayMessage;							// Message d'absence

		bool _errorMsgTooLongSent;							// Message d'erreur envoyé si le message est trop long
		bool _pingSent;										// Indique si le serveur attend un PONG du client

		std::map<std::string, Channel*> _channelsJoined;	// Liste des canaux auxquels le client est connecté

	public:

		Client(int fd);
		~Client();

		// === SETTERS INFOS CLIENT ===
		void setNickname(const std::string &nickname);						// Définit le pseudo du client
		void setUsername(const std::string &username);						// Définit le nom d'utilisateur
		void setRealName(const std::string &realName);						// Définit le nom réel
		void setHostname(const std::string &hostname);						// Définit le nom d'hôte
		void setClientIp(const std::string &clientIp);						// Définit l'adresse IP client
		void setUsermask();													// Définit le usermask du client pour RPL
		void setClientPort(int clientPort);									// Définit le port client

		void setIsIrssi(bool status);										// Définit si le client est un Irssi
		void setIdentified(bool status);									// Définit si irssi fournit les nick et user par indent
		void setIdentNickCmd(std::vector<std::string> identCmd);			// Définit la commande d'identification nickname d'Irssi
		void setIdentUsernameCmd(std::vector<std::string> identCmd);		// Définit la commande d'identification username d'Irssi
		void setServPasswordValidity(bool status);							// Définit si le mot de passe est valide
		void authenticate();												// Authentifie le client
		
		void setLastActivity();
		void setIsAway(bool status); 										// Définit si le client est absent
		void setAwayMessage(const std::string& message); 					// Définit le message d'absence du client
		void setErrorMsgTooLongSent(bool status);							// Définit si le message d'erreur d'un input trop long est déjà envoyé
		void setPingSent(bool status);										// Définit si le serveur attend un PONG du client
		
		// === BUFFER ===
		std::string& getBufferMessage();									// Récupère le buffer de message

		// === GETTERS INFOS CLIENT ===
		int getFd() const;													// Récupère le descripteur de socket du client
		const std::string& getNickname() const;								// Récupère le pseudo
		const std::string& getUsername() const;								// Récupère le nom d'utilisateur
		const std::string& getRealName() const;								// Récupère le nom réel
		const std::string& getHostname() const;								// Récupère le nom d'hôte
		const std::string& getClientIp() const;								// Récupère l'adresse IP client
		const std::string& getUsermask() const;								// Récupère le usermask du client pour RPL
		int getClientPort() const;											// Récupère le port client

		bool isIrssi() const;												// Vérifie si le client est un Irssi
		bool isIdentified() const;											// Vérifie si irssi fournit les nick et user par ident
		std::vector<std::string> getIdentNickCmd() const;					// Récupère la commande d'identification nickname d'Irssi
		std::vector<std::string> getIdentUsernameCmd() const;				// Récupère la commande d'identification username d'Irssi
		bool gotValidServPassword() const;									// Vérifie si le client a donné le bon mot de passe du serveur
		bool isAuthenticated() const;										// Vérifie si le client est authentifié
		
		time_t getSignonTime() const;										// Récupère le timestamp de connexion
		time_t getLastActivity() const;										// Récupère le dernier moment actif du client
		time_t getIdleTime() const; 										// Récupère le temps d'inactivité du client
		bool isAway() const;												// Vérifie si le client est absent
		const std::string& getAwayMessage() const;							// Récupère le message d'absence
		bool errorMsgTooLongSent() const;									// Vérifie si le message d'erreur d'un input trop long est déjà envoyé
		bool pingSent() const;												// Dit si le serveur attend un PONG du client

		// === SEND MESSAGES ===
		void sendMessage(const std::string &message, Client* sender) const;						// Le serveur envoie un message au client
		void sendToAll(Channel* channel, const std::string &message, bool includeSender);		// Envoie un message formaté irc à tous les clients connectés a un channel

		// === GETTERS CHANNELS ===
		std::map<std::string, Channel*>& getChannelsJoined();				// Récupère les canaux auxquels le client est connecté
		bool isInChannel(const std::string& channelName) const;				// Vérifie si le client est membre d'un canal
		bool isOperator(Channel* channel) const;							// Vérifie si le client est un opérateur sur un canal
		bool isInvited(const Channel* channel) const;						// Vérifie si le client est invité sur un canal

		// === UPDATE CHANNELS ===
		void joinChannel(const std::string& channelName, const std::string& password, std::map<std::string, Channel*>& channels);						// Rejoint un canal (creer et rejoindre ou rejoindre existant)
		void createChannel(const std::string& channelName, const std::string& password, std::map<std::string, Channel*>& channels);						// Crée un canal
		void addToChannel(Channel* channel, const std::string& password, const std::string& channelName, std::map<std::string, Channel*>& channels);	// Ajoute un client à un canal
		void msgAfterJoin(Channel* channel, const std::string& channelName); 																			// Send les bons RPL IRC après un join channel
		bool hasRightPassword(Channel* channel, const std::string& password);																			// Vérifie le mot de passe du canal
		void passwordSetting(Channel* channel, const std::string& password);																			// Définit le mot de passe du canal
		void isKickedFromChannel(Channel *channel, Client* kicker, const std::string& reason);															// Est exclu d'un canal
		void isInvitedToChannel(Channel *channel, const Client* inviter);
		
		void leaveChannel(std::map<std::string, Channel*>::iterator it, std::map<std::string, Channel*>& channels, const std::string& reason, int reasonCode);		// Quitte un canal
		void leaveAllChannels(std::map<std::string, Channel*>& channels, const std::string& reason, int reasonCode);												// Quitte tous les canaux
		void deleteChannel(Channel* channel, std::map<std::string, Channel*>& channels);																			// Supprime un canal
};