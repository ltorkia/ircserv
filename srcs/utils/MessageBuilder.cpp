/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageBuilder.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 19:32:30 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MessageBuilder.hpp"

// === OTHER CLASSES ===
#include "Utils.hpp"

// === NAMESPACES ===
#include "irc_config.hpp"
#include "irc_replies.hpp"
#include "commands.hpp"
#include "colors.hpp"

using namespace irc_replies;
using namespace commands;
using namespace colors;

// ========================================= PRIVATE =======================================

MessageBuilder::MessageBuilder() {}
MessageBuilder::MessageBuilder(const MessageBuilder& src) {(void) src;}
MessageBuilder& MessageBuilder::operator=(const MessageBuilder& src) {(void) src; return *this;}
MessageBuilder::~MessageBuilder() {}


// ========================================= PUBLIC ========================================

// === CLIENT SIDE ===

// =========================================================================================

// ========= NOTICES / UTILS =========

// Si le message est trop long on le coupe, et on rajoute \r\n
std::string MessageBuilder::ircFormat(const std::string& message)
{
	if (message.length() > server::BUFFER_SIZE)
		return message.substr(0, server::BUFFER_SIZE) + eol::IRC;
	return message + eol::IRC;
}

// Format utilisé pour les messages qui n'ont pas de code spécifique
// mais qui doivent être interprétés par Irssi sans erreur.
// -- Deux versions: avant et après authentification client
std::string MessageBuilder::ircNoticeMsg(const std::string& message, const std::string& colorCode)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " NOTICE * :" << colorCode << message << IRC_RESET;
	return stream.str();
}
std::string MessageBuilder::ircNoticeMsg(const std::string& nickname, const std::string& message, const std::string& colorCode)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " NOTICE " << nickname << " :" << colorCode << message << IRC_RESET;
	return stream.str();
}


// === EXCEPTIONS ===

std::string MessageBuilder::ircClientException(const std::exception &e)
{
	return ircNoticeMsg(server::NAME, std::string(e.what()), IRC_COLOR_ERR);
}


// ========= RPL / IRC FORMATTED MESSAGES =========

// === PING -> PONG ===

std::string MessageBuilder::ircPing(void)
{
	std::ostringstream stream;
	stream << PING << " :" << server::NAME;
	return stream.str();
}
std::string MessageBuilder::ircPong(void)
{
	std::ostringstream stream;
	stream << PONG << " :" << server::NAME;
	return stream.str();
}

// === HANDLE CAPABILITIES ===

std::string MessageBuilder::ircCapabilities(const std::string& arg)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << CAP << " * " << arg << " :";
	return stream.str();
}


// === AUTHENTICATION PROMPT ===

std::string MessageBuilder::ircCommandPrompt(const std::string& commandPrompt, const std::string& prevCommand)
{
	std::ostringstream stream;
	std::vector<std::string> commands = Utils::getTokens(commandPrompt, splitter::COMMA);
	std::vector<std::string>::iterator cmd = commands.begin();

	if (!prevCommand.empty())
	{
		stream << ":" << server::NAME << " NOTICE " << server::NAME 
		<< " :" << IRC_COLOR_ERR << prevCommand << " "
		<< IRC_COLOR_PROMPT << *cmd << IRC_RESET << eol::IRC;
		return stream.str();
	}

	stream << ":" << server::NAME << " NOTICE " << server::NAME << " :" << IRC_COLOR_PROMPT << "Please enter:" << IRC_RESET << eol::IRC;
	while (cmd != commands.end())
	{
		stream << ":" << server::NAME << " NOTICE " << server::NAME << " :" << IRC_COLOR_PROMPT << *cmd << IRC_RESET;
		if (cmd != --commands.end())
			stream << eol::IRC;
		cmd++;
	}
	return stream.str();
}

std::string MessageBuilder::ircUsernameSet(const std::string& username)
{
	return ircNoticeMsg(server::NAME, "Username successfully set to "  + IRC_DEFAULT + "'" + username + "'", IRC_COLOR_SUCCESS);
}

std::string MessageBuilder::ircFirstNicknameSet(const std::string& nickname)
{
	return ircNoticeMsg(server::NAME, "Nickname successfully set to "  + IRC_DEFAULT + "'" + nickname + "'", IRC_COLOR_SUCCESS);
}

std::string MessageBuilder::ircChangingNickname(const std::string& nickname)
{
	return ircNoticeMsg(server::NAME, "Nickname " + IRC_DEFAULT + "'" + nickname + "'" + IRC_COLOR_ERR + " already registered. Renaming...", IRC_COLOR_ERR);
}


// === RPL CONNECT ===

// --- 001 RPL_WELCOME : Message de bienvenue après une connexion réussie.
std::string MessageBuilder::ircWelcomeMessage(const std::string& nickname, const std::string& usermask)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_WELCOME << " " << nickname 
	<< " :" << IRC_COLOR_INFO << RPL_WELCOME_MSG << IRC_RESET << " " << usermask;
	return stream.str();
}

std::string MessageBuilder::ircMOTDMessage(const std::string& nickname)
{
	std::ostringstream stream;

	// --- 375 RPL_MOTDSTART : Début du message du jour
	stream << ":" << server::NAME << " " << RPL_MOTDSTART << " " << nickname 
	<< " :" << IRC_PURPLE << "-- " << server::NAME << " " << RPL_MOTDSTART_MSG << " --" << IRC_RESET << eol::IRC;
	
	// --- 372 RPL_MOTD : Message du jour
	stream << ":" << server::NAME << " " << RPL_MOTD << " " << nickname 
	<< " :" << IRC_COLOR_DISPLAY << RPL_MOTD_MSG << IRC_RESET << eol::IRC;
	
	// --- 376 RPL_ENDOFMOTD : Fin du message du jour
	stream << ":" << server::NAME << " " << RPL_ENDOFMOTD << " " << nickname 
	<< " :" << IRC_PURPLE << "-- " << RPL_ENDOFMOTD_MSG << " --" << IRC_RESET;

	return stream.str();
}

// --- 002 RPL_YOURHOST : Retourne le nom et la version du serveur auquel le client est connecté.
std::string MessageBuilder::ircHostInfos(const std::string& nickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_YOURHOST << " " << nickname 
	<< " :" << IRC_COLOR_INFO << "Host: " << server::NAME << " | Version: " << server::VERSION << IRC_RESET;
	return stream.str();
}

// --- 003 RPL_CREATED : Retourne la date de création du serveur.
std::string MessageBuilder::ircTimeCreation(const std::string& nickname, const std::string& serverCreationTime)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_CREATED << " " << nickname 
	<< " :" << IRC_COLOR_INFO << RPL_CREATED_MSG << " " << serverCreationTime << IRC_RESET;
	return stream.str();
}

// --- 004 RPL_MYINFO : Informations sur le serveur et modes disponibles.
std::string MessageBuilder::ircInfos(const std::string& nickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_MYINFO << " " << nickname 
	<< " :" << IRC_COLOR_INFO << RPL_MYINFO_MSG << IRC_RESET;
	return stream.str();
}

std::string MessageBuilder::ircGlobalUserList(const std::string& nickname, int userCount, int knownCount, int unknownCount, int channelCount)
{
	std::ostringstream stream;

	// Nombre d'utilisateurs inconnus (pas encore identifiés)
	stream << ":" << server::NAME << " " << RPL_LUSERUNKNOWN << " " << nickname
	<< " :Unknown connections: " << unknownCount << eol::IRC;

	// Nombre de canaux actifs
	stream << ":" << server::NAME << " " << RPL_LUSERCHANNELS << " " << nickname
	<< " :Channels: " << channelCount << eol::IRC;

	// Nombre total d'utilisateurs sur le serveur
	stream << ":" << server::NAME << " " << RPL_LUSERME << " " << nickname
	<< " :Online users: " << userCount << " (" << knownCount << " authenticated)" << eol::IRC;

	return stream.str();
}

// --- 431 ERR_NONICKNAMEGIVEN : Aucun nickname fourni.
std::string MessageBuilder::ircNoNicknameGiven(const std::string& nickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_NONICKNAMEGIVEN << " " << nickname 
	<< " :" << IRC_COLOR_ERR << ERR_NONICKNAMEGIVEN_MSG << IRC_RESET;
	return stream.str();
}

// --- 432 ERR_ERRONEUSNICKNAME : Le pseudonyme a des caracteres non autorisés.
std::string MessageBuilder::ircErroneusNickname(const std::string& nickname, const std::string& enteredNickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_ERRONEUSNICKNAME << " " << nickname << " " << enteredNickname
	<< " :" << IRC_COLOR_ERR << ERR_ERRONEUSNICKNAME_MSG << IRC_RESET;
	return stream.str();
}

// --- 433 ERR_NICKNAMEINUSE : Le pseudonyme choisi est déjà utilisé.
std::string MessageBuilder::ircNicknameTaken(const std::string& nickname, const std::string& enteredNickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_NICKNAMEINUSE << " " << nickname << " " << enteredNickname 
	<< " :" << ERR_NICKNAMEINUSE_MSG;
	return stream.str();
}

// --- 464 ERR_PASSWDMISMATCH : Mot de passe incorrect.
std::string MessageBuilder::ircPasswordIncorrect(void)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_PASSWDMISMATCH 
	<< " * :" << IRC_COLOR_ERR << ERR_PASSWDMISMATCH_MSG << IRC_RESET;
	return stream.str();
}

// 462 ERR_ALREADYREGISTRED : Le client est deja enregistre.
std::string MessageBuilder::ircAlreadyRegistered(const std::string& nickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_ALREADYREGISTERED << " " << nickname 
	<< " :" << IRC_COLOR_ERR << ERR_ALREADYREGISTERED_MSG << IRC_RESET;
	return stream.str();
}


// === RPL MESSAGES ===

// Message vers channel
std::string MessageBuilder::ircMsgToChannel(const std::string& nickname, const std::string& channelName, const std::string& message)
{
	std::ostringstream stream;
	stream << ":" << nickname << " " << PRIVMSG << " " << channelName << " :" << message;
	return stream.str();
}
// Message privé client to client
std::string MessageBuilder::ircMsgToClient(const std::string& nickname, const std::string& receiverName, const std::string& message)
{
	std::ostringstream stream;	
	stream << ":" << nickname << " " << PRIVMSG << " " << receiverName << " :" << message;
	return stream.str();
}
// Message envoyé aux autres clients d'un channel quand un client join ce channel
std::string MessageBuilder::ircClientJoinChannel(const std::string& usermask, const std::string& channelName)
{
	std::ostringstream stream;	
	stream << ":" << usermask << " " << JOIN << " :" << channelName;
	return stream.str();
}
// Message envoyé aux autres clients d'un channel quand un opérateur change un mode
std::string MessageBuilder::ircOpeChangedMode(const std::string& usermask, const std::string& channelName, const std::string& changedMode, const std::string& parameter)
{
	std::ostringstream stream;	
	std::string param = !parameter.empty() ? " " + parameter : "";
	stream << ":" << usermask << " " << MODE << " " << channelName << " " << changedMode << param;
	return stream.str();
}
// Message envoyé aux autres clients d'un channel quand le topic est change
std::string MessageBuilder::ircTopicMessage(const std::string& usermask, const std::string& channelName, const std::string& topic)
{
	std::ostringstream stream;
	stream << ":" << usermask << " " << TOPIC << " " << channelName << " :" << topic;
	return stream.str();
}
// Message envoyé aux autres clients d'un channel quand un client est kick
std::string MessageBuilder::ircClientKickUser(const std::string& usermask, const std::string& channelName, const std::string& kickedUser, const std::string& reason)
{
	std::ostringstream stream;
	std::string givenReason = !reason.empty() ? " :" + reason : "";
	stream << ":" << usermask << " " << KICK << " " << channelName << " " << kickedUser << givenReason;
	return stream.str();
}
// Message envoyé aux autres clients d'un channel quand un client s'en va
std::string MessageBuilder::ircClientPartChannel(const std::string& usermask, const std::string& channelName, const std::string& reason)
{
	std::ostringstream stream;
	std::string givenReason = !reason.empty() ? " :" + reason : "";
	stream << ":" << usermask << " " << PART << " " << channelName << givenReason;
	return stream.str();
}
// Message envoyé aux autres clients d'un channel quand un client quitte le serveur
std::string MessageBuilder::ircClientQuitServer(const std::string& usermask, const std::string& message)
{
	std::ostringstream stream;	
	stream << ":" << usermask << " " << QUIT << " :Quit: " << message;
	return stream.str();
}
// Message d'erreur envoyé au client avant de fermer sa connexion
std::string MessageBuilder::ircErrorQuitServer(const std::string& reason)
{
	std::ostringstream stream;
	stream << "ERROR :" << reason;
	return stream.str();
}


// === RPL CHANNELS ===

std::string MessageBuilder::ircNameReply(const std::string& nickname, const std::string& channelName, const std::string& users)
{
	std::ostringstream stream;
	// --- 353 RPL_NAMREPLY : Liste des utilisateurs présents dans un canal.
	stream	<< ":" << server::NAME << " " << RPL_NAMREPLY << " " << nickname << " = " << channelName << " :" << users << eol::IRC;
	// --- 366 RPL_ENDOFNAMES : Fin de la liste des utilisateurs pour un canal.
	stream	<< ":" << server::NAME << " " << RPL_ENDOFNAMES << " " << nickname << " " << channelName << " :" << RPL_ENDOFNAMES_MSG;
	return stream.str();
}

// --- 331 RPL_NOTOPIC : Aucun sujet défini pour le canal.
std::string MessageBuilder::ircNoTopic(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_NOTOPIC << " " << nickname << " " << channelName
	<< " :" << IRC_COLOR_INFO << RPL_NOTOPIC_MSG << IRC_RESET;
	return stream.str();
}

// --- 332 RPL_TOPIC : Sujet actuel du canal.
std::string MessageBuilder::ircTopic(const std::string& nickname, const std::string& channelName, const std::string& topic)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_TOPIC << " " << nickname << " " << channelName
		<< " :" << IRC_COLOR_INFO << topic << IRC_RESET;
	return stream.str();
}

// --- 333 RPL_TOPICWHOTIME : Informations supplémentaires sur qui a défini le sujet et à quelle heure.
std::string MessageBuilder::ircTopicWhoTime(const std::string& nickname, const std::string& setterNick, const std::string& channelName, time_t topicTime)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_TOPICWHOTIME << " " << nickname << " " << channelName << " " << setterNick
	<< " " << topicTime;
	return stream.str();
}

// --- 403 ERR_NOSUCHCHANNEL : Le canal spécifié n'existe pas.
std::string MessageBuilder::ircNoSuchChannel(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_NOSUCHCHANNEL << " " << nickname << " " << channelName 
	<< " :" << IRC_COLOR_ERR << ERR_NOSUCHCHANNEL_MSG << IRC_RESET;
	return stream.str();
}

// --- 404 ERR_CANNOTSENDTOCHAN : Le message n'a pas pu etre delivre au canal.
std::string MessageBuilder::ircCannotSendToChan(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_CANNOTSENDTOCHAN << " " << nickname << " " << channelName 
	<< " :" << IRC_COLOR_ERR << ERR_CANNOTSENDTOCHAN_MSG << IRC_RESET;
	return stream.str();
}

// --- 411 ERR_NORECIPIENT : Pas de destinataire.
std::string MessageBuilder::ircNoRecipient(const std::string& nickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_NORECIPIENT << " " << nickname 
	<< " :" << IRC_COLOR_ERR << ERR_NORECIPIENT_MSG << IRC_RESET;
	return stream.str();
}

// --- 412 ERR_NOTEXTTOSEND : Aucun message à envoyer.
std::string MessageBuilder::ircNoTextToSend(const std::string& nickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_NOTEXTTOSEND << " " << nickname 
	<< " :" << IRC_COLOR_ERR << ERR_NOTEXTTOSEND_MSG << IRC_RESET;
	return stream.str();
}

// --- 417 ERR_INPUTTOOLONG : Message trop long.
std::string MessageBuilder::ircLineTooLong(const std::string& nickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_INPUTTOOLONG << " " << nickname 
	<< " :" << IRC_COLOR_ERR << ERR_INPUTTOOLONG_MSG << IRC_RESET;
	return stream.str();
}

// --- 441 ERR_USERNOTINCHANNEL : L'utilisateur cible n'est pas dans le canal.
std::string MessageBuilder::ircNotInChannel(const std::string& nickname, const std::string& channelName, const std::string &targetNick)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_USERNOTINCHANNEL << " " << nickname << " " << targetNick << " " << channelName 
	<< " :" << IRC_COLOR_ERR << ERR_USERNOTINCHANNEL_MSG << IRC_RESET;
	return stream.str();
}

// --- 442 ERR_NOTONCHANNEL : Le présent utilisateur n'est pas dans le canal.
std::string MessageBuilder::ircCurrentNotInChannel(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_NOTONCHANNEL << " " << nickname << " " << channelName 
	<< " :" << IRC_COLOR_ERR << ERR_NOTONCHANNEL_MSG << IRC_RESET;
	return stream.str();
}

// --- 341 RPL_INVITING : L'utilisateur a été invité dans le canal.
std::string MessageBuilder::ircInviting(const std::string& nickname, const std::string& targetNick, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_INVITING << " " << nickname << " " << targetNick << " " << channelName;
	return stream.str();
}

// 471 ERR_CHANNELISFULL : Channel full.
std::string MessageBuilder::ircChannelFull(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_CHANNELISFULL << " " << nickname << " " << channelName
	<< " :" << IRC_COLOR_ERR << ERR_CHANNELISFULL_MSG << IRC_RESET;
	return stream.str();
}

// --- 443 ERR_USERONCHANNEL : Quand un client invité au canal est déja dans le canal.
std::string MessageBuilder::ircAlreadyOnChannel(const std::string& nickname, const std::string& targetNick, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_USERONCHANNEL << " " << nickname << " " << targetNick << " " << channelName 
	<< " :" << IRC_COLOR_ERR << ERR_USERONCHANNEL_MSG << IRC_RESET;
	return stream.str();
}

// --- 465 ERR_YOUREBANNEDCREEP : L'utilisateur est banni du serveur.
std::string MessageBuilder::ircBannedFromServer(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_YOUREBANNEDCREEP << " " << nickname << " " << channelName
	<< " :" << IRC_COLOR_ERR << ERR_YOUREBANNEDCREEP_MSG << IRC_RESET;
	return stream.str();
}

// --- 472 ERR_UNKNOWNMODE : Mode inconnu.
std::string MessageBuilder::ircUnknownMode(const std::string& nickname, char character)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_UNKNOWNMODE << " " << nickname << " " << character
	<< " :" << IRC_COLOR_ERR << ERR_UNKNOWNMODE_MSG << IRC_RESET;
	return stream.str();
}

// --- 473 ERR_INVITEONLYCHAN : Le canal est en mode invitation (+i), et l'utilisateur n'est pas invité.
std::string MessageBuilder::ircInviteOnly(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_INVITEONLYCHAN << " " << nickname << " " << channelName
	<< " :" << IRC_COLOR_ERR << ERR_INVITEONLYCHAN_MSG << IRC_RESET;
	return stream.str();
}

// --- 474 ERR_BANNEDFROMCHAN : L'utilisateur est banni (+b) du canal.
std::string MessageBuilder::ircBannedFromChannel(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_BANNEDFROMCHAN << " " << nickname << " " << channelName
	<< " :" << IRC_COLOR_ERR << ERR_BANNEDFROMCHAN_MSG << IRC_RESET;
	return stream.str();
}

// --- 475 ERR_BADCHANNELKEY : Mauvais mot de passe pour rejoindre le canal.
std::string MessageBuilder::ircWrongChannelPass(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_BADCHANNELKEY << " " << nickname << " " << channelName
	<< " :" << IRC_COLOR_ERR << ERR_BADCHANNELKEY_MSG << IRC_RESET;
	return stream.str();
}

// --- 476 ERR_BADCHANMASK : Nom du channel mal formaté ou invalide (#).
std::string MessageBuilder::ircBadChannelName(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_BADCHANMASK << " " << nickname << " " << channelName 
	<< " :" << IRC_COLOR_ERR << ERR_BADCHANMASK_MSG << IRC_RESET;
	return stream.str();
}

// --- 477 ERR_NEEDREGGEDNICK : Certains serveurs nécessitent un pseudo enregistré (+r).
std::string MessageBuilder::ircNeedNick(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_NEEDREGGEDNICK << " " << nickname << " " << channelName
	<< " :" << IRC_COLOR_ERR << ERR_NEEDREGGEDNICK_MSG << IRC_RESET;
	return stream.str();
}

// --- 482 ERR_CHANOPRIVSNEEDED : L’utilisateur n'est pas opérateur et essaie une action nécessitant des droits d'opérateur.
std::string MessageBuilder::ircNotChanOperator(const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_CHANOPRIVSNEEDED << " " << channelName 
	<< " :" << IRC_COLOR_ERR << ERR_CHANOPRIVSNEEDED_MSG << IRC_RESET;
	return stream.str();
}


// === NOTICE CHANNELS ===

std::string MessageBuilder::ircInvitedToChannel(const std::string& nickname, const std::string& channelName)
{
	return ircNoticeMsg(server::NAME, IRC_DEFAULT + nickname + IRC_COLOR_INFO + " invited you to join channel " + IRC_DEFAULT + channelName, IRC_COLOR_INFO);
}
std::string MessageBuilder::ircAlreadyInvitedToChannel(const std::string& nickname, const std::string& channelName)
{
	return ircNoticeMsg(server::NAME, IRC_DEFAULT + nickname + IRC_COLOR_INFO + " has already been invited to join channel " + IRC_DEFAULT + channelName, IRC_COLOR_INFO);
}
std::string MessageBuilder::ircNoPassNeeded(const std::string& channelName)
{
	return ircNoticeMsg(server::NAME, "No pass needed to join channel " + IRC_DEFAULT + channelName, IRC_COLOR_INFO);
}


// === RPL CLIENTS ===

std::string MessageBuilder::ircNicknameSet(const std::string& oldNickname, const std::string& newNickname)
{
	std::string oldNick = !oldNickname.empty() ? oldNickname : "unknown";
	std::ostringstream stream;
	stream << ":" << oldNickname << " " << NICK << " " << newNickname;
	return stream.str();
}

// --- 401 ERR_NOSUCHNICK : Le pseudonyme spécifié n'existe pas.
std::string MessageBuilder::ircNoSuchNick(const std::string& nickname, const std::string& targetNick)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_NOSUCHNICK << " " << nickname << " " << targetNick 
	<< " :" << IRC_COLOR_ERR << ERR_NOSUCHNICK_MSG << IRC_RESET;
	return stream.str();
}

// --- 301 RPL_AWAY : L'utilisateur est absent.
std::string MessageBuilder::ircClientIsAway(const std::string& nickname, const std::string& targetNick, const std::string& message)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_AWAY << " " << nickname << " "
	<< targetNick << " :" << message;
	return stream.str();
}

// --- 305 RPL_UNAWAY : L'utilisateur n'est plus absent.
std::string MessageBuilder::ircUnAway(const std::string& nickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_UNAWAY << " " << nickname << " :" << RPL_UNAWAY_MSG;
	return stream.str();
}

// --- 306 RPL_NOWAWAY : L'utilisateur est maintenant absent.
std::string MessageBuilder::ircAway(const std::string& nickname)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_NOWAWAY << " " << nickname << " :" << RPL_NOWAWAY_MSG;
	return stream.str();
}

std::string MessageBuilder::ircWhois(const std::string& nickname, const std::string& targetNick, const std::string& username, 
	const std::string& realname, const std::string& clientIp)
{
	std::ostringstream stream;
	
	// RPL_WHOISUSER (311) : Informations de base sur l'utilisateur
	stream << ":" << server::NAME << " " << RPL_WHOISUSER << " " << nickname << " "
	<< targetNick << " " << username << " " << clientIp << " * :" << realname << eol::IRC;

	// RPL_WHOISSERVER (312) : Serveur de l'utilisateur
	stream << ":" << server::NAME << " " << RPL_WHOISSERVER << " " << nickname << " "
	<< targetNick << " " << server::NAME << " :" << RPL_WHOISSERVER_MSG << eol::IRC;

	return stream.str();
}

// RPL_WHOISUSER (311) : Temps d'inactivité de l'utilisateur.
std::string MessageBuilder::ircWhoisIdle(const std::string& nickname, const std::string& targetNick, time_t idleTime, time_t signonTime)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_WHOISIDLE << " " << nickname << " "
	<< targetNick << " " << idleTime << " " << signonTime << " :" << RPL_WHOISIDLE_MSG << eol::IRC;
	return stream.str();
}

// RPL_ENDOFWHOIS (318) : Fin du WHOIS
std::string MessageBuilder::ircEndOfWhois(const std::string& nickname, const std::string& targetNick)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_ENDOFWHOIS << " " << nickname << " " 
	<< targetNick << " :" << RPL_ENDOFWHOIS_MSG << eol::IRC;
	return stream.str();
}

// RPL_WHOREPLY (352) : Liste des utilisateurs du canal
std::string MessageBuilder::ircWho(const std::string& nickname, const std::string& targetNick, const std::string& username, 
	const std::string& realname, const std::string& clientIp, const std::string& channelName, bool isAway)
{
	std::string awayChar = isAway ? "G" : "H";
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_WHOREPLY << " " << nickname << " " 
	<< channelName << " " << username << " " << clientIp << " " << server::NAME << " " 
	<< targetNick << " " << awayChar << " :" << 0 << " " << realname << eol::IRC;
	return stream.str();
}

// RPL_ENDOFWHO (315) : Fin de la liste
std::string MessageBuilder::ircEndOfWho(const std::string& nickname, const std::string& channelName)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_ENDOFWHO << " " << nickname << " " 
	<< channelName << " :" << RPL_ENDOFWHO_MSG << eol::IRC;
	return stream.str();
}

// RPL_ENDOFWHOWAS (369) : Fin du WHOIS
std::string MessageBuilder::ircEndOfWhowas(const std::string& nickname, const std::string& targetNick)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_ENDOFWHOWAS << " " << nickname << " " 
	<< targetNick << " :" << RPL_ENDOFWHOWAS_MSG << eol::IRC;
	return stream.str();
}


// === RPL COMMAND ERRORS ===

// --- 421 ERR_UNKNOWNCOMMAND : La commande n'est pas reconnue par le serveur.
std::string MessageBuilder::ircUnknownCommand(const std::string& nickname, const std::string& command)
{
	std::string cmd = Utils::truncateStr(Utils::streamArg(command));
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_UNKNOWNCOMMAND << " " << nickname << " " << cmd 
	<< " :" << IRC_COLOR_ERR << ERR_UNKNOWNCOMMAND_MSG << IRC_RESET;
	return stream.str();
}
	
// 461 ERR_NEEDMOREPARAMS : Il manque des paramètres pour une commande.
std::string MessageBuilder::ircNeedMoreParams(const std::string& nickname, const std::string& command)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_NEEDMOREPARAMS << " " << nickname << " " << command 
	<< " :" << IRC_COLOR_ERR << ERR_NEEDMOREPARAMS_MSG << IRC_RESET;
	return stream.str();
}

// 451 ERR_NOTREGISTERED : L'utilisateur doit être enregistré avant de pouvoir exécuter des commandes.
std::string MessageBuilder::ircNotRegistered(void)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_NOTREGISTERED 
	<< " :" << IRC_COLOR_ERR << ERR_NOTREGISTERED_MSG << IRC_RESET;
	return stream.str();
}


// === RPL MODES ===

// 324 RPL_CHANNELMODEIS :Sent to a client to inform them of the currently-set modes of a channel. <channel> is the name of the channel. <modestring> and <mode arguments> 
// are a mode string and the mode arguments (delimited as separate parameters) as defined in the MODE message description.
std::string MessageBuilder::ircChannelModeIs(const std::string& nickname, const std::string& channel, const std::string& displaymode)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_CHANNELMODEIS << " " << nickname << " " << channel << " " << displaymode;
	return stream.str();
}
// :server 324 <nickname> <channel> <modes> <mode_params>

// 329 RPL_CREATIONTIME : Sent to a client to inform them of the creation time of a channel. <creationtime> is a unix timestamp representing when the channel was created on the network.
std::string MessageBuilder::ircCreationTime(const std::string& nickname, const std::string& channel, time_t time)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_CREATIONTIME << " " << nickname << " " << channel << " " << time;
	return stream.str();
}

// 696 ERR_INVALIDMODEPARAM : Indicates that there was a problem with a mode parameter. Replaces various implementation-specific mode-specific numerics.
std::string MessageBuilder::ircInvalidModeParams(const std::string &nickname, const std::string& channel, const std::string& mode_char, const std::string &param)
{
	std::string parameter = Utils::truncateStr(Utils::streamArg(param));
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_INVALIDMODEPARAM << " " << nickname << " " << channel << " "
	<< mode_char << " " << parameter << " :" << IRC_COLOR_ERR << ERR_INVALIDMODEPARAM_MSG << IRC_RESET;
	return stream.str();
}

// 367 RPL_BANLIST : Sent as a reply to the MODE command, when clients are viewing the current entries on a channel’s ban list. 
std::string MessageBuilder::ircBannedList(const std::string &nickname, const std::string &channel, const std::string &who, time_t time_channel) //ptet pas le bon chan
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_BANLIST << " " << IRC_COLOR_INFO << nickname << " " << channel << " *!*@* " << who << " " << time_channel << IRC_RESET;
	return stream.str();
}

// 368 RPL_ENDOFBANLIST : Sent as a reply to the MODE command, this numeric indicates the end of a channel’s ban list.
std::string MessageBuilder::ircEndOfBannedList(const std::string &nickname, const std::string &channel)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << RPL_ENDOFBANLIST << " " << nickname << " " << channel
	<< " :" << RPL_ENDOFBANLIST_MSG;
	return stream.str();
}

// 525 ERR_INVALIDKEY : Indicates the value of a key channel mode change (+k) was rejected.
std::string MessageBuilder::ircInvalidPasswordFormat(const std::string &nickname, const std::string& channel)
{
	std::ostringstream stream;
	stream << ":" << server::NAME << " " << ERR_INVALIDKEY  << " " << nickname << " " << channel
	<< " :" << IRC_COLOR_ERR << ERR_INVALIDKEY_MSG << IRC_RESET;
	return stream.str();
}


// =========================================================================================

// === SERVER SIDE ===

// =========================================================================================

// === DEFAULT MESSAGE FORMAT ===

/**
 * @brief This function constructs a formatted message string using the provided color, message, and end-of-line (EOL) characters.
 *
 * @param color The color to be used for the message.
 * @param message The main content of the message.
 * @param eol The end-of-line character(s) to be appended at the end of the message.
 *
 * @return A string containing the formatted message.
 */
std::string MessageBuilder::msgBuilder(const std::string& color, const std::string& message, const std::string& eol)
{
	std::ostringstream stream;
	stream << color << message << RESET << eol;
	return stream.str();
}

// === EXCEPTIONS ===

std::string MessageBuilder::msgServerException(const std::exception &e)
{
	std::ostringstream stream;
	stream << COLOR_ERR << "Error: " << RESET << e.what() << eol::UNIX;
	return stream.str();
}


// === DISPLAY ===

void MessageBuilder::displayWelcome(const std::string &serverIp, int port, const std::string &password)
{
	std::cout << std::endl << CYAN << "==================================================" << RESET << std::endl << std::endl;
	std::cout << GREEN << "            ██╗" << RED << "     ██████╗ " << BLUE << "   ██████╗ " << RESET << std::endl;
	std::cout << GREEN << "            ██║" << RED << "     ██╔═══╝ " << BLUE << "  ██╔════╝ " << RESET << std::endl;
	std::cout << GREEN << "            ██║" << RED << "     ██║     " << BLUE << "  ██║      " << RESET << std::endl;
	std::cout << GREEN << "            ██║" << RED << "     ██║     " << BLUE << "  ██║      " << RESET << std::endl;
	std::cout << GREEN << "            ██║" << RED << "     ██║     " << BLUE << "  ╚██████╗ " << RESET << std::endl;
	std::cout << GREEN << "            ╚═╝" << RED << "     ╚═╝     " << BLUE << "   ╚═════╝ " << RESET << std::endl << std::endl;
	std::cout << GREEN << "          Intense    " << RED << "Récré    " << BLUE << "Cérébrale" << RESET << std::endl << std::endl;
	std::cout << CYAN << "==================================================" << RESET << std::endl << std::endl;
	std::cout << BLUE << "         IRC server successfully launched !" << RESET << std::endl;
	std::cout << PURPLE << "         Waiting for connections... " << RESET << std::endl << std::endl;
	std::cout << RED << "         🖥️  - IP Server: " << WHITE << serverIp << RESET << std::endl;
	std::cout << RED << "         🔌 - Port:      " << WHITE << port << RESET << std::endl;
	std::cout << RED << "         🔑 - Password:  " << WHITE << password << RESET << std::endl;
	std::cout << std::endl << CYAN << "==================================================" << RESET << std::endl << std::endl;
}

/**
 * @brief Get the current local time as a formatted string.
 * 
 * This function retrieves the current local time and formats it into a string
 * in the format "YYYY-MM-DD at HH:MM:SS".
 * 
 * @return std::string The formatted current local time.
 */
std::string MessageBuilder::msgServerCreationTime()
{
	std::tm now;
	Utils::getCurrentTime(now);

	std::ostringstream stream;
	stream 
		<< (now.tm_year + 1900) << "-"		// Année
		<< (now.tm_mon + 1) << "-"			// Mois (de 0 à 11, donc +1)
		<< now.tm_mday << " at "			// Jour
		<< now.tm_hour << ":"				// Heure
		<< now.tm_min << ":"				// Minute
		<< now.tm_sec;						// Seconde

	return stream.str();
}


// === SETTINGS ===

std::string MessageBuilder::msgSignalCaught(const std::string& signalType)
{
	return msgBuilder(COLOR_ERR, signalType + " signal caught, server shutting down...", eol::UNIX);
}


// === CLIENTS ===

std::string MessageBuilder::msgClientConnected(const std::string& clientIp, int port, int socket, const std::string& nickname)
{
	
	std::string text;
	std::string nickDisplay;

	if (!nickname.empty())
	{
		nickDisplay = DEFAULT + nickname + " ";
		text = "👤 " + nickDisplay + COLOR_SUCCESS + "registered";
	} else
		text = "➕ " + COLOR_INFO + "New unregistered client";

	std::ostringstream stream;
	stream << text << " => " << COLOR_DISPLAY << "[" << clientIp << "][port " << port << "][socket " << socket << "]" << RESET;
	return stream.str();
}
std::string MessageBuilder::msgClientDisconnected(const std::string& clientIp, int port, int socket, const std::string& nickname)
{
	
	std::string text;
	std::string nickDisplay;

	if (!nickname.empty())
	{
		nickDisplay = DEFAULT + nickname + " ";
		text = "👋 " + nickDisplay + COLOR_ERR + "left";
	} else
		text = "➖ " + COLOR_INFO + "Unregistered client left";

	std::ostringstream stream;
	stream << text << " => " << COLOR_DISPLAY << "[" << clientIp << "][port " << port << "][socket " << socket << "]" << RESET;
	return stream.str();
}


// === CHANNELS ===

std::string MessageBuilder::msgClientCreatedChannel(const std::string& nickname, const std::string& channelName, const std::string& password)
{
	std::string notifPass = !password.empty() ? COLOR_SUCCESS + " with password set" : "";
	return msgBuilder("🚀 " + COLOR_SUCCESS, DEFAULT + nickname + COLOR_SUCCESS + " created channel " + DEFAULT + channelName + notifPass, "");
}
std::string MessageBuilder::msgIsInvitedToChannel(const std::string& nickname, const std::string& inviterNick, const std::string& channelName)
{
	return msgBuilder("📩 " + COLOR_INFO, DEFAULT + inviterNick + COLOR_INFO + " invited " + DEFAULT + nickname + COLOR_INFO + " to join channel " + DEFAULT + channelName, "");
}
std::string MessageBuilder::msgClientJoinedChannel(const std::string& nickname, const std::string& channelName)
{
	return msgBuilder("➡️  " + COLOR_SUCCESS, DEFAULT + nickname + COLOR_SUCCESS + " joined channel " + DEFAULT + channelName, "");
}
std::string MessageBuilder::msgClientSetTopic(const std::string& nickname, const std::string& channelName, const std::string& topic)
{
	std::string notifTopic = topic.empty() ? "NO TOPIC" : topic;
	return msgBuilder("📢 " + COLOR_INFO, DEFAULT + nickname + COLOR_INFO + " set topic on channel " + DEFAULT + channelName + COLOR_INFO + " to " + DEFAULT + notifTopic, "");
}
std::string MessageBuilder::msgClientOperatorAdded(const std::string& nickname, const std::string& channelName)
{
	return msgBuilder("🛡️  " + COLOR_INFO, DEFAULT + nickname + COLOR_INFO + " is now operator of channel " + DEFAULT + channelName, "");
}
std::string MessageBuilder::msgClientOperatorRemoved(const std::string& nickname, const std::string& channelName)
{
	return msgBuilder("⚠️  " + COLOR_INFO, DEFAULT + nickname + COLOR_INFO + " doesn't operate on channel " + DEFAULT + channelName + COLOR_INFO + " anymore", "");
}
std::string MessageBuilder::msgClientLeftChannel(const std::string& nickname, const std::string& channelName, const std::string& reason)
{
	return msgBuilder("⬅️  " + COLOR_ERR, DEFAULT + nickname + COLOR_ERR + " left channel " + DEFAULT + channelName + PURPLE + " (" + reason + ")", "");
}
std::string MessageBuilder::msgClientKickedFromChannel(const std::string& nickname, const std::string& kickerNick, const std::string& channelName, const std::string& reason)
{
	return msgBuilder("⛔ " + COLOR_INFO, DEFAULT + kickerNick + COLOR_INFO + " kicked " + DEFAULT + nickname + COLOR_INFO + " from channel " + DEFAULT + channelName + PURPLE + " (" + reason + ")", "");
}
std::string MessageBuilder::msgNoClientInChannel(const std::string& channelName)
{
	return msgBuilder("🏜️  " + COLOR_INFO, "No client left in channel " + DEFAULT + channelName, "");
}
std::string MessageBuilder::msgChannelDestroyed(const std::string& channelName)
{
	return msgBuilder("💥 " + COLOR_ERR, "Channel " + DEFAULT + channelName + COLOR_ERR + " destroyed.", "");
}


// === FILES ===

std::string MessageBuilder::msgFileUsage(const std::string& subCommand)
{
	std::string clientType = subCommand == file::SEND_CMD ? "receiver" : "sender";
	std::ostringstream stream;
	stream << "Usage: DCC " << subCommand << " <" << clientType << "> <file_path>";
	return stream.str();
}

std::string MessageBuilder::msgSendFile(const std::string& filename, const std::string& sender, const std::string& adr, const int& port)
{
	std::ostringstream stream;
	stream << "📤 " << DCC << " SEND FROM " << sender << " [" << adr << " " << port << "]: " << filename;
	return stream.str();
}

std::string MessageBuilder::msgRequestSent(const std::string& filename, const std::string& receiver)
{
	std::ostringstream stream;
	stream << DCC << " SEND request sent to " << receiver << ": " << filename;
	return stream.str();
}

std::string MessageBuilder::msgSendingFile(const std::string& filename, const std::string& receiver)
{
	std::ostringstream stream;
	stream << DCC << " sending file " << filename << " for " << receiver;
	return stream.str();
}

std::string MessageBuilder::msgFileReceived(const std::string& filename, const std::string& sender)
{
	std::ostringstream stream;
	stream << DCC << " received file " << filename << " from " << sender;
	return stream.str();
}

std::string MessageBuilder::msgFileSent(const std::string& filename, const std::string& receiver)
{
	std::ostringstream stream;
	stream << DCC << " sent file " << filename << " for " << receiver;
	return stream.str();
}

std::string MessageBuilder::errorMsgOpenFile(const std::string& path)
{
	std::ostringstream stream;
	stream << DCC << " can't open file " << path << ": No such file or directory";
	return stream.str();
}

std::string MessageBuilder::errorMsgWriteFile(const std::string& path)
{
	std::ostringstream stream;
	stream << DCC << " can't write file " << path;
	return stream.str();
}

std::string MessageBuilder::errorMsgNoFile(const std::string& sender)
{
	std::ostringstream stream;
	stream << DCC << " no file offered by " << sender;
	return stream.str();
}

// =========================================================================================

// === BOT ===

// =========================================================================================

std::string MessageBuilder::botGetAge(int years, int months, int days)
{
	std::ostringstream stream;
	stream << "⏳ " << "You are " << years << " years, " << months << " months, and " << days << " days old !";
	return stream.str();
}

// Message privé bot to client
std::string MessageBuilder::botCmdPrivmsg(const std::string& targetName, const std::string& message)
{
	std::ostringstream stream;	
	stream << PRIVMSG << " " << targetName << " :" << message;
	return stream.str();
}

// Bot join channel
std::string MessageBuilder::botCmdJoinChannel(const std::string& channelName)
{
	std::ostringstream stream;	
	stream << JOIN << " " << channelName;
	return stream.str();
}