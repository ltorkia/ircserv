#pragma once

#include <iostream>				// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <sstream>				// gestion flux -> std::ostringstream
#include <set>					// container set

// =========================================================================================

class Client;
class Channel
{
	private:
		Channel();
		Channel(const Channel& src);
		Channel& operator=(const Channel& src);

		std::string _name, _password;								// Nom et mot de passe du canal
		std::string _topic, _topicSetterMask;						// Sujet du canal et auteur de la dernière modification du topic
		time_t _channelTimestamp, _topicTimestamp;					// Moment où a ete cree le channel et date de la dernière modification du sujet format UNIX

		std::set<const Client*> _connected, _operators, _invited;	// Liste des clients connectés, des opérateurs et des invités	
		
		bool _invites, _rightsTopic;								// Modes invite +i et topic +t
		int _limits;												// Mode +l limite de clients sur le canal
	
	public:
		Channel(const std::string &name, const std::string& password);
		~Channel();

		// === CHANNEL PROPERTIES SETTERS === Channel_Properties.cpp
		void setPassword(const std::string &password);						// Définit le mot de passe du canal
		void setTopic(const std::string &topic);							// Définit le sujet du canal
		void setTopicSetterMask(const std::string& setterUsermask);			// Définit l'auteur de la dernière modification du sujet
		void setTopicTimestamp();											// Définit la date de la dernière modification du sujet
		void topicSettings(const std::string& topic, const Client* setter); // Actions suivants la modification du topic

		void setInvites(const bool info);
		void setRightsTopic(const bool info);
		void setLimits(const int info);
		
		// === CHANNEL PROPERTIES GETTERS === Channel_Properties.cpp
		const std::string& getName() const;					// Récupère le nom du canal
		const std::string& getPassword() const;				// Récupère le mot de passe du canal
		const std::string& getTopic() const;				// Récupère le sujet du canal
		const std::string& getTopicSetterMask() const;		// Récupère l'auteur de la dernière modification du sujet
		time_t getTopicTimestamp() const;					// Récupère la date de la dernière modification du sujet
		time_t getCreationTime() const;						// Récupère le creation time du canal
		std::string getMode() const; 						// Récupère les modes du canal

		int getConnectedCount() const;
		std::set<const Client*> getClientsList() const;		// Récupère la liste des clients du canal
		std::set<const Client*> getOperatorsList() const;	// Récupère la liste des operators du canal
		std::set<const Client*> getInvitedList() const;		// Récupère la liste des clients invités sur le canal
		std::string getNicknames() const;					// Récupère la liste des pseudos des clients connectés au canal
		int getChannelClientByNickname(const std::string &nickname, const Client* currClient); // Récupère le client du canal par son pseudo

		bool getInvites() const;
		bool getRightsTopic() const;
		int getLimits() const;
		
		bool hasClients() const;							// Vérifie si le canal a des clients
		bool hasOperators() const;							// Vérifie si le canal a des operators
		bool hasInvites() const;							// Vérifie si le canal a des invitations en cours
		bool hasPassword() const;							// Vérifie si le canal a un mot de passe
		bool hasTopic() const;								// Vérifie si le canal a un sujet
		bool isFull() const;								// Vérifie si le canal est plein
		
		bool isConnected(const Client* client) const;		// Vérifie si un client spécifique est connecté au canal
		bool isOperator(const Client* client) const;		// Vérifie si un client spécifique est un operator du canal
		bool isInvited(const Client* client) const;			// Vérifie si un client spécifique est invité sur le canal

		// === CHANNEL ACTIONS === Channel_Actions.cpp
		void addClient(Client* client);							// Ajoute un client au canal
		void addClientToInvitedList(const Client* invited,
									const Client* inviter);		// Ajoute un client a la liste d'invitation
		void addOperator(Client* client);						// Ajoute un operator au canal
		void removeOperator(Client* client);					// Retire un operator du canal
		void removeClient(Client* client, const Client* kicker,
				const std::string& reason, int reasonCode);		// Retire un client du canal
		
		void sendToAll(const std::string &message, Client* sender, bool includeSender);	// Envoie un message à tous les clients connectés du canal
};