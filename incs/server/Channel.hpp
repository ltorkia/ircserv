#pragma once

#include <iostream>				// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <sstream>				// gestion flux -> std::ostringstream
#include <set>					// container set

// =========================================================================================

class Client;
class Channel
{
	private:
		// =================================================================================
		
		// === VARIABLES ===

		// =================================================================================

		Channel();
		Channel(const Channel& src);
		Channel& operator=(const Channel& src);

		std::string _name, _password;								// Nom et mot de passe du canal
		std::string _topic, _topicSetterMask;						// Sujet du canal et auteur de la dernière modification du topic
		time_t _channelTimestamp, _topicTimestamp;					// Moment où a ete cree le channel et date de la dernière modification du sujet format UNIX

		std::set<const Client*> _connected, _operators, _invited;	// Liste des clients connectés, des opérateurs et des invités	
		
		bool _isInviteOnly, _isSettableTopic;						// Modes invite +i et topic +t
		int _clientsLimit;											// Mode +l limite de clients sur le canal
	
	public:
		// =================================================================================
		// === CHANNEL CONSTRUCTOR / DESTRUCTOR === Channel.cpp

		Channel(const std::string &name, const std::string& password);
		~Channel();

		// =================================================================================
		
		// === PUBLIC METHODS ===

		// =================================================================================

		// =================================================================================
		// === CHANNEL ATTRIBUTES SETTERS === Channel_Attributes.cpp

		// === MODES SETTINGS ===
		void setPassword(const std::string &password);						// Définit le mot de passe du canal
		void setClientsLimit(const int state);								// Définit la limite de clients sur le canal
		void setInviteOnly(const bool state);								// Définit le mode invite-only du canal
		void setSettableTopic(const bool state); 							// Définit si le client peut modifier le sujet du canal
		
		// === TOPIC SETTINGS ===
		void topicSettings(const std::string& topic, const Client* setter); // Actions suivants la modification du topic
		void setTopic(const std::string &topic);							// Définit le sujet du canal
		void setTopicSetterMask(const std::string& setterUsermask);			// Définit l'auteur de la dernière modification du sujet
		void setTopicTimestamp();											// Définit la date de la dernière modification du sujet
		
		// =================================================================================
		// === CHANNEL ATTRIBUTES GETTERS === Channel_Attributes.cpp

		// === CHANNEL INFOS ===
		time_t getCreationTime() const;						// Récupère le creation time du canal
		const std::string& getName() const;					// Récupère le nom du canal
		std::string getModes() const; 						// Récupère les modes du canal

		// === MODES CHECK + GETTER ===
		bool hasPassword() const;							// Vérifie si le canal a un mot de passe
		bool hasClientsLimit() const;						// Vérifie si le canal a une limite de clients
		bool isInviteOnly() const;							// Vérifie si le canal est en mode invite-only
		bool isSettableTopic() const;						// Vérifie si le client peut modifier le sujet du canal

		// === MODES VALUES ===
		const std::string& getPassword() const;				// Récupère le mot de passe du canal
		int getClientsLimit() const; 						// Récupère la limite de clients sur le canal

		// === TOPIC CHECK + GETTERS ===
		bool hasTopic() const;								// Vérifie si le canal a un sujet
		const std::string& getTopic() const;				// Récupère le sujet du canal
		const std::string& getTopicSetterMask() const;		// Récupère l'auteur de la dernière modification du sujet
		time_t getTopicTimestamp() const;					// Récupère la date de la dernière modification du sujet

		// === CLIENTS CHECK ===
		bool isFull() const;								// Vérifie si le canal est plein
		bool hasClients() const;							// Vérifie si le canal a des clients
		bool hasInvitedClients() const;						// Vérifie si le canal a des invitations
		bool hasOperators() const;							// Vérifie si le canal a des operators

		// === CLIENTS LISTS ===
		std::set<const Client*> getClientsList() const;		// Récupère la liste des clients du canal
		std::set<const Client*> getInvitedList() const;		// Récupère la liste des clients invités sur le canal
		std::set<const Client*> getOperatorsList() const;	// Récupère la liste des operators du canal
		
		int getConnectedCount() const;						// Récupère le nombre de clients connectés au canal
		int getChannelClientByNickname(const std::string &nickname, const Client* currClient); // Récupère le client du canal par son pseudo
		std::string getNicknames() const;					// Récupère la liste des pseudos des clients connectés au canal

		bool isConnected(const Client* client) const;		// Vérifie si un client spécifique est connecté au canal
		bool isOperator(const Client* client) const;		// Vérifie si un client spécifique est un operator du canal
		bool isInvited(const Client* client) const;			// Vérifie si un client spécifique est invité sur le canal

		// =================================================================================
		// === ACTIONS === Channel_Actions.cpp
		
		// === CLIENT MANAGER ===
		void addClient(Client* client);							// Ajoute un client au canal
		void addClientToInvitedList(const Client* invited,
									const Client* inviter);		// Ajoute un client a la liste d'invitation
		void addOperator(Client* client);						// Ajoute un operator au canal
		void removeOperator(Client* client);					// Retire un operator du canal
		void removeClient(Client* client, const Client* kicker,
				const std::string& reason, int reasonCode);		// Retire un client du canal
		
		// === MESSAGES ===
		void sendToAll(const std::string &message, Client* sender, bool includeSender);	// Envoie un message à tous les clients connectés du canal
};