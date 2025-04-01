/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc_replies.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:38:09 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include "irc_config.hpp"

namespace irc_replies
{
	// === USER STATS ===
	const std::string RPL_LUSEROP 					= "252";	// Nombre d'opérateurs en ligne
	const std::string RPL_LUSERUNKNOWN 				= "253";	// Nombre d'utilisateurs inconnus (non enregistrés)
	const std::string RPL_LUSERCHANNELS 			= "254";	// Nombre de canaux actifs
	const std::string RPL_LUSERME 					= "255";	// Résumé des utilisateurs et opérateurs sur le serveur


	// === CONNECT ===

	// 001 RPL_WELCOME : Message de bienvenue après une connexion réussie.
	const std::string RPL_WELCOME 					= "001";
	const std::string RPL_WELCOME_MSG				= "Welcome to the Internet Relay Network";

	// 002 RPL_YOURHOST : Retourne le nom et la version du serveur auquel le client est connecté.
	const std::string RPL_YOURHOST 					= "002";

	// 003 RPL_CREATED : Retourne la date de création du serveur.
	const std::string RPL_CREATED 					= "003";
	const std::string RPL_CREATED_MSG 				= "Server created on";

	// 004 RPL_MYINFO : Informations sur le serveur et modes disponibles.
	const std::string RPL_MYINFO 					= "004";
	const std::string RPL_MYINFO_MSG 				= "Available channel modes: itkol";

	// 375 RPL_MOTDSTART : Début du message du jour
	const std::string RPL_MOTDSTART 				= "375";
	const std::string RPL_MOTDSTART_MSG 			= "Message of the day";

	// 372 RPL_MOTD : Message du jour
	const std::string RPL_MOTD 						= "372";
	const std::string RPL_MOTD_MSG 					= "Gimme dat smile =)";

	// 376 RPL_ENDOFMOTD : Fin du message du jour
	const std::string RPL_ENDOFMOTD 				= "376";
	const std::string RPL_ENDOFMOTD_MSG 			= "End of MOTD";

	// 431 ERR_NONICKNAMEGIVEN : Aucun nickname fourni.
	const std::string ERR_NONICKNAMEGIVEN 			= "431";
	const std::string ERR_NONICKNAMEGIVEN_MSG 		= "No nickname given";

	// 432 ERR_ERRONEUSNICKNAME : Le pseudonyme a des caracteres non autorisés.
	const std::string ERR_ERRONEUSNICKNAME 			= "432";
	const std::string ERR_ERRONEUSNICKNAME_MSG 		= "Erroneus nickname";

	// 433 ERR_NICKNAMEINUSE : Le pseudonyme choisi est déjà utilisé.
	const std::string ERR_NICKNAMEINUSE 			= "433";
	const std::string ERR_NICKNAMEINUSE_MSG 		= "This nickname is already taken";

	// 464 ERR_PASSWDMISMATCH : Le mot de passe fourni pour l'authentification d'un utilisateur ne correspond pas à celui attendu par le serveur.
	const std::string ERR_PASSWDMISMATCH 			= "464";
	const std::string ERR_PASSWDMISMATCH_MSG 		= "Incorrect password";


	// === CHANNELS ===

	// 331 RPL_NOTOPIC : Aucun sujet défini pour le canal.
	const std::string RPL_NOTOPIC 					= "331";
	const std::string RPL_NOTOPIC_MSG 				= "No topic is set";

	// 332 RPL_TOPIC : Sujet actuel du canal.
	const std::string RPL_TOPIC 					= "332";

	// 333 RPL_TOPICWHOTIME
	const std::string RPL_TOPICWHOTIME 				= "333";

	// 352 RPL_WHOREPLY : 
	const std::string RPL_WHOREPLY 					= "352";

	// 315 RPL_ENDOFWHO
	const std::string RPL_ENDOFWHO 					= "315";
	const std::string RPL_ENDOFWHO_MSG             = "End of /WHO list";

	// 341 RPL_INVITING : L'utilisateur a été invité dans le canal.
	const std::string RPL_INVITING 					= "341";

	// 353 RPL_NAMREPLY : Liste des utilisateurs présents dans un canal.
	const std::string RPL_NAMREPLY 					= "353";

	// 366 RPL_ENDOFNAMES : Fin de la liste des utilisateurs pour un canal.
	const std::string RPL_ENDOFNAMES 				= "366";
	const std::string RPL_ENDOFNAMES_MSG 			= "End of /NAMES list";

	// 403 ERR_NOSUCHCHANNEL : Le canal spécifié n'existe pas.
	const std::string ERR_NOSUCHCHANNEL 			= "403";
	const std::string ERR_NOSUCHCHANNEL_MSG 		= "No such channel";

	// 404 ERR_CANNOTSENDTOCHAN : Le message n'a pas pu etre delivre au canal.
	const std::string ERR_CANNOTSENDTOCHAN 			= "404";
	const std::string ERR_CANNOTSENDTOCHAN_MSG 		= "Cannot send to channel";

	// 411 ERR_NORECIPIENT : Pas de destinataire.
	const std::string ERR_NORECIPIENT 				= "411";
	const std::string ERR_NORECIPIENT_MSG 			= "No recipient given (PRIVMSG)";

	// 412 ERR_NOTEXTTOSEND : Aucun message à envoyer.
	const std::string ERR_NOTEXTTOSEND 				= "412";
	const std::string ERR_NOTEXTTOSEND_MSG 			= "No text to send";

	// 417 ERR_INPUTTOOLONG : Message trop long.
	const std::string ERR_INPUTTOOLONG 				= "417";
	const std::string ERR_INPUTTOOLONG_MSG 			= "Input line too long, message truncated";

	// 441 ERR_USERNOTINCHANNEL : L'utilisateur visé n'est pas dans le canal.
	const std::string ERR_USERNOTINCHANNEL 			= "441";
	const std::string ERR_USERNOTINCHANNEL_MSG 		= "User not in channel";

	// 442 ERR_NOTONCHANNEL : Le présent utilisateur n'est pas dans le canal.
	const std::string ERR_NOTONCHANNEL 				= "442";
	const std::string ERR_NOTONCHANNEL_MSG 			= "You're not on this channel";

	// 443 ERR_USERONCHANNEL : Quand un client invité au canal est déja dans le canal.
	const std::string ERR_USERONCHANNEL 			= "443";
	const std::string ERR_USERONCHANNEL_MSG 		= "is already on channel";

	// 465 ERR_YOUREBANNEDCREEP : L'utilisateur est banni du serveur.
	const std::string ERR_YOUREBANNEDCREEP 			= "465";
	const std::string ERR_YOUREBANNEDCREEP_MSG 		= "You're banned from this server";

	// 471 ERR_CHANNELISFULL : Le canal est plein (+l limite atteinte).
	const std::string ERR_CHANNELISFULL 			= "471";
	const std::string ERR_CHANNELISFULL_MSG 		= "Channel is full";

	// 472 ERR_UNKNOWNMODE : Mode inconnu.
	const std::string ERR_UNKNOWNMODE 				= "472";
	const std::string ERR_UNKNOWNMODE_MSG 			= "Mode unknown";

	// 473 ERR_INVITEONLYCHAN : Le canal est en mode invitation (+i), et l'utilisateur n'est invité.
	const std::string ERR_INVITEONLYCHAN 			= "473";
	const std::string ERR_INVITEONLYCHAN_MSG 		= "Invite only channel";

	// 474 ERR_BANNEDFROMCHAN : L'utilisateur est banni (+b) du canal.
	const std::string ERR_BANNEDFROMCHAN 			= "474";
	const std::string ERR_BANNEDFROMCHAN_MSG 		= "You're banned from this channel";

	// 475 ERR_BADCHANNELKEY : Mauvais mot de passe pour rejoindre le canal.
	const std::string ERR_BADCHANNELKEY 			= "475";
	const std::string ERR_BADCHANNELKEY_MSG 		= "Incorrect password";

	// 476 ERR_BADCHANMASK : Nom du channel mal formaté ou invalide (#).
	const std::string ERR_BADCHANMASK 				= "476";
	const std::string ERR_BADCHANMASK_MSG 			= "Bad channel mask";

	// 477 ERR_NEEDREGGEDNICK : Certains serveurs nécessitent un pseudo enregistré (+r).
	const std::string ERR_NEEDREGGEDNICK 			= "477";
	const std::string ERR_NEEDREGGEDNICK_MSG 		= "You must be registered to join this channel";

	// 482 ERR_CHANOPRIVSNEEDED : L’utilisateur n'est pas opérateur et essaie une action nécessitant des droits d'opérateur.
	const std::string ERR_CHANOPRIVSNEEDED 			= "482";
	const std::string ERR_CHANOPRIVSNEEDED_MSG 		= "You're not channel operator";

	//525 ERR_INVALIDKEY : Indicates the value of a key channel mode change (+k) was rejected.
	const std::string ERR_INVALIDKEY 				= "525";
	const std::string ERR_INVALIDKEY_MSG 			= "Key is not well-formed";


	// === CLIENTS ===

	// 301 RPL_AWAY : L'utilisateur est absent.
	const std::string RPL_AWAY 						= "301";
	const std::string RPL_AWAY_MSG 					= "is away";

	// 305 RPL_UNAWAY : L'utilisateur n'est plus absent.
	const std::string RPL_UNAWAY 					= "305";
	const std::string RPL_UNAWAY_MSG 				= "You are no longer marked as being away";

	// 306 RPL_NOWAWAY : L'utilisateur est maintenant absent.
	const std::string RPL_NOWAWAY 					= "306";
	const std::string RPL_NOWAWAY_MSG 				= "You have been marked as being away";

	// 311 RPL_WHOISUSER : Informations de base sur un utilisateur (via WHOIS).
	const std::string RPL_WHOISUSER 				= "311";

	// 312 RPL_WHOISSERVER : Serveur de l'utilisateur.
	const std::string RPL_WHOISSERVER 				= "312";
	const std::string RPL_WHOISSERVER_MSG			= "server info";

	// 317 RPL_WHOISIDLE : Temps d'inactivité de l'utilisateur.
	const std::string RPL_WHOISIDLE 				= "317";
	const std::string RPL_WHOISIDLE_MSG				= "seconds idle, signon time";

	// 318 RPL_ENDOFWHOIS : Fin de la commande WHOIS.
	const std::string RPL_ENDOFWHOIS 				= "318";
	const std::string RPL_ENDOFWHOIS_MSG 			= "End of /WHOIS list";

	// 369 RPL_ENDOFWHOWAS : Fin de la commande WHOWAS.
	const std::string RPL_ENDOFWHOWAS 				= "369";
	const std::string RPL_ENDOFWHOWAS_MSG			= "End of /WHOWAS list";

	// 401 ERR_NOSUCHNICK : Le pseudonyme spécifié n'existe pas.
	const std::string ERR_NOSUCHNICK 				= "401";
	const std::string ERR_NOSUCHNICK_MSG 			= "Nickname not found";


	// === COMMAND ERRORS ===

	// 421 ERR_UNKNOWNCOMMAND : La commande n'est pas reconnue par le serveur.
	const std::string ERR_UNKNOWNCOMMAND 			= "421";
	const std::string ERR_UNKNOWNCOMMAND_MSG 		= "Unknown command";

	// 461 ERR_NEEDMOREPARAMS : Parametre invalide.
	const std::string ERR_NEEDMOREPARAMS 			= "461";
	const std::string ERR_NEEDMOREPARAMS_MSG 		= "Invalid parameters";

	// 451 ERR_NOTREGISTERED : L'utilisateur doit être enregistré avant de pouvoir exécuter des commandes.
	const std::string ERR_NOTREGISTERED 			= "451";
	const std::string ERR_NOTREGISTERED_MSG 		= "Please register first";

	// 462 ERR_ALREADYREGISTRED : Le client est deja enregistre
	const std::string ERR_ALREADYREGISTERED 		= "462";
	const std::string ERR_ALREADYREGISTERED_MSG 	= "You are already registered";


	// === MODE ===

	// 324 RPL_CHANNELMODEIS : pas de mode donne pour le channel
	const std::string RPL_CHANNELMODEIS 			= "324";

	// 329  RPL_CREATIONTIME : donne quand a ete cree le channel
	const std::string RPL_CREATIONTIME 				= "329";

	// 696 ERR_INVALIDMODEPARAM
	const std::string ERR_INVALIDMODEPARAM 			= "696";
	const std::string ERR_INVALIDMODEPARAM_MSG		= "int required only";

	// 367 RPL_BANLIST
	const std::string RPL_BANLIST 					= "367";

	// 368 RPL_ENDOFBANLIST
	const std::string RPL_ENDOFBANLIST 				= "368";
	const std::string RPL_ENDOFBANLIST_MSG 			= "End of channel ban list";
}