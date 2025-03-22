#pragma once

#include <iostream>			// gestion chaînes de caractères -> std::cout, std::cerr, std::string
#include <sstream>			// gestion flux -> std::ostringstream
#include <ctime> 			// gestion temps -> std::time_t, std::tm
#include <vector>			// container vector

// === NAMESPACES ===
#include "../config/irc_config.hpp"
#include "../config/irc_replies.hpp"
#include "../config/commands.hpp"
#include "../config/colors.hpp"

// === CLASSES ===
#include "Utils.hpp"

// =========================================================================================

class MessageHandler {

	private:
		MessageHandler();
		MessageHandler(const MessageHandler& src);
		MessageHandler & operator=(const MessageHandler& src);
		~MessageHandler();

	public:

		/**************************** CLIENT SIDE ****************************/

		// === BUILDER UTILS ===
		static std::string ircFormat(const std::string& message);
		static std::string ircBasicMsg(const std::string& message, const std::string& colorCode);
		static std::string ircBasicMsg(const std::string& nickname, const std::string& message, const std::string& colorCode);
		
		// === FORMAT EXCEPTIONS ===
		static std::string ircClientException(const std::exception &e);

		// === PING -> PONG ===
		static std::string ircPing(void);
		static std::string ircPong(void);

		// === HANDLE CAPABILITIES ===
		static std::string ircCapabilities(const std::string& arg);

        // === AUTHENTICATION PROMPT ===
		static std::string ircCommandPrompt(const std::string& commandPrompt, const std::string& prevCommand, bool error);
		static std::string ircUsernameSet(const std::string& username);
		static std::string ircFirstNicknameSet(const std::string& nickname);
		static std::string ircChangingNickname(const std::string& nickname);
		
		// === CONNECT ===
		static std::string ircWelcomeMessage(const std::string& nickname, const std::string& usermask);
		static std::string ircMOTDMessage(const std::string& nickname);
		static std::string ircHostInfos(const std::string& nickname);
		static std::string ircTimeCreation(const std::string& nickname, const std::string& serverCreationTime);
		static std::string ircInfos(const std::string& nickname);
		static std::string ircGlobalUserList(const std::string& nickname, int userCount, int knownCount, int unknownCount, int channelCount);
		static std::string ircNoNicknameGiven(const std::string& nickname);
		static std::string ircErroneusNickname(const std::string& nickname, const std::string& enteredNickname);
		static std::string ircNicknameTaken(const std::string& nickname, const std::string& enteredNickname);
		static std::string ircPasswordIncorrect(void);
		static std::string ircAlreadyRegistered(const std::string& nickname);

		// === RPL MESSAGES ===
		static std::string ircMsgToChannel(const std::string& nickname, const std::string& channelName, const std::string& message);
		static std::string ircMsgToClient(const std::string& nickname, const std::string& receiverName, const std::string& message);
		static std::string ircClientJoinChannel(const std::string& usermask, const std::string& channelName);
		static std::string ircOpeChangedMode(const std::string& usermask, const std::string& channelName, const std::string& changedMode, const std::string& parameter);
		static std::string ircTopicMessage(const std::string& usermask, const std::string& channelName, const std::string& topic);
		static std::string ircClientKickUser(const std::string& usermask, const std::string& channelName, const std::string& kickedUser, const std::string& reason);
		static std::string ircClientPartChannel(const std::string& usermask, const std::string& channelName, const std::string& reason);
		static std::string ircClientQuitServer(const std::string& usermask, const std::string& message);
		static std::string ircErrorQuitServer(const std::string& reason);

		// === RPL CHANNELS ===
		static std::string ircNameReply(const std::string& nickname, const std::string& channelName, const std::string& users);
		static std::string ircNoTopic(const std::string& nickname, const std::string& channelName);
		static std::string ircTopic(const std::string& nickname, const std::string& channelName, const std::string& topic);
		static std::string ircTopicWhoTime(const std::string& nickname, const std::string& setterNick, const std::string& channelName, time_t topicTime);
		static std::string ircNoSuchChannel(const std::string& nickname, const std::string& channelName);
		static std::string ircCannotSendToChan(const std::string& nickname, const std::string& channelName);
		static std::string ircNoRecipient(const std::string& nickname);
		static std::string ircNoTextToSend(const std::string& nickname);
		static std::string ircLineTooLong(const std::string& nickname);
		static std::string ircNotInChannel(const std::string& nickname, const std::string& channelName, const std::string &targetNick);
		static std::string ircCurrentNotInChannel(const std::string& nickname, const std::string& channelName);
		static std::string ircInviting(const std::string& nickname, const std::string& targetNick, const std::string& channelName);
		static std::string ircAlreadyOnChannel(const std::string& nickname, const std::string& targetNick, const std::string& channelName);
		static std::string ircBannedFromServer(const std::string& nickname, const std::string& channelName);
		static std::string ircChannelFull(const std::string& nickname, const std::string& channelName);
		static std::string ircUnknownMode(const std::string& nickname, char character);
		static std::string ircInviteOnly(const std::string& nickname, const std::string& channelName);
		static std::string ircBannedFromChannel(const std::string& nickname, const std::string& channelName);
		static std::string ircWrongChannelPass(const std::string& nickname, const std::string& channelName);
		static std::string ircBadChannelName(const std::string& nickname, const std::string& channelName);
		static std::string ircNeedNick(const std::string& nickname, const std::string& channelName);
		static std::string ircNotChanOperator(const std::string& channelName);

		// === NOTICE CHANNELS ===
		static std::string ircChannelCreated(const std::string& nickname, const std::string& channelName);
		static std::string ircChannelDestroyed(const std::string& channelName);
		static std::string ircOperatorAdded(const std::string& nickname, const std::string& channelName);
		static std::string ircOperatorRemoved(const std::string& nickname, const std::string& channelName);
		static std::string ircInvitedToChannel(const std::string& nickname, const std::string& channelName);
		static std::string ircAlreadyInvitedToChannel(const std::string& nickname, const std::string& channelName);
		static std::string ircNoPassNeeded(const std::string& channelName);

		// === CLIENTS ===
		static std::string ircNicknameSet(const std::string& oldNickname, const std::string& newNickname);
		static std::string ircNoSuchNick(const std::string& nickname, const std::string& targetNick);
		static std::string ircClientIsAway(const std::string& nickname, const std::string& targetNick, const std::string& message);
		static std::string ircUnAway(const std::string& nickname);
		static std::string ircAway(const std::string& nickname);
		static std::string ircWhois(const std::string& nickname, const std::string& targetNick, const std::string& username, const std::string& realname, const std::string& clientIp);
		static std::string ircWhoisIdle(const std::string& nickname, const std::string& targetNick, time_t idleTime, time_t signonTime);
		static std::string ircEndOfWhois(const std::string& nickname, const std::string& targetNick); 
		static std::string ircWho(const std::string& nickname, const std::string& targetNick, const std::string& username, const std::string& realname, const std::string& clientIp, const std::string& channelName, bool isAway);
		static std::string ircEndOfWho(const std::string& nickname, const std::string& channelName);
		static std::string ircEndOfWhowas(const std::string& nickname, const std::string& targetNick);

		// === COMMAND ERRORS ===
		static std::string ircUnknownCommand(const std::string& nickname, const std::string& command);
		static std::string ircNeedMoreParams(const std::string& nickname, const std::string& command);
		static std::string ircNotRegistered(void);

		// === MODE ===
		static std::string ircChannelModeIs(const std::string& nickname, const std::string& channel, const std::string& displaymode);
		static std::string ircCreationTime(const std::string& nickname, const std::string& channel, time_t time);
		static std::string ircInvalidModeParams(const std::string &nickname, const std::string& channel, const std::string& mode_char, const std::string&param);
		static std::string ircInvalidPasswordFormat(const std::string &nickname, const std::string& channel);
		static std::string ircBannedList(const std::string &nickname, const std::string &channel, const std::string &who, time_t time_channel);
		static std::string ircEndOfBannedList(const std::string &nickname, const std::string &channel);

		
		/**************************** SERVER SIDE ****************************/

		static std::string msgBuilder(const std::string& color, const std::string& message, const std::string& eol);

		// === EXCEPTIONS ===
		static std::string msgServerException(const std::exception &e);

		// === DISPLAY ===
		static void displayWelcome(const std::string &serverIp, int port, const std::string &password);
		static std::string msgTimeServerCreation();

		// === SETTINGS ===
		static std::string msgSignalCaught(const std::string& signalType);
		
		// === CLIENTS ===
		static std::string msgClientConnected(const std::string& clientIp, int port, int socket, const std::string& nickname);
		static std::string msgClientDisconnected(const std::string& clientIp, int port, int socket, const std::string& nickname);

		// === CHANNELS ===
		static std::string msgClientCreatedChannel(const std::string& nickname, const std::string& channelName, const std::string& password);
		static std::string msgIsInvitedToChannel(const std::string& nickname, const std::string& inviterNick, const std::string& channelName);
		static std::string msgClientJoinedChannel(const std::string& nickname, const std::string& channelName);
		static std::string msgClientSetTopic(const std::string& nickname, const std::string& channelName, const std::string& topic);
		static std::string msgClientOperatorAdded(const std::string& nickname, const std::string& channelName);
		static std::string msgClientOperatorRemoved(const std::string& nickname, const std::string& channelName);
		static std::string msgClientLeftChannel(const std::string& nickname, const std::string& channelName, const std::string& reason);
		static std::string msgClientKickedFromChannel(const std::string& nickname, const std::string& kickerNick, const std::string& channelName, const std::string& reason);
		static std::string msgNoClientInChannel(const std::string& channelName);
		static std::string msgChannelDestroyed(const std::string& channelName);


		/**************************** BONUS ****************************/
		
		static std::string msgSendFile(const std::string& filename, const std::string &client, const std::string &adr, const int &port);
		static std::string errorMsgSendFile(const std::string& filename);
		static std::string msgSendingFile(const std::string& filename, const std::string& receiver, const std::string& ip, const int &port);
		static std::string botGetAge(int years, int months, int days);
};