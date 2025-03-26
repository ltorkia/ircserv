#include "../../../incs/server/CommandHandler.hpp"

// === OTHER CLASSES ===
#include "../../../incs/utils/Utils.hpp"
#include "../../../incs/utils/IrcHelper.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/irc_config.hpp"
#include "../../../incs/config/commands.hpp"

using namespace commands;
using namespace error_display;

// =========================================================================================

// === MODE PARSER ===

/**
 * @brief Changes the mode of a channel or user.
 *
 * This function parses the mode change command, validates the arguments,
 * and executes the appropriate mode change operations.
 *
 * The function performs the following steps:
 * 1. Tokenizes the input command to extract the target (channel or user) and mode.
 * 2. Validates the command and checks for errors.
 * 3. Determines the mode arguments and executes the corresponding mode change functions.
 *
 * @note The function assumes that the input command is stored in the member variable `_itInput`.
 *
 * @return void
 */
void CommandHandler::_changeMode()
{
	std::string mode;
	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::WORD);
	std::string cible = *args.begin();
	if (args.size() > 1)
		mode = *++args.begin();
	if (_validateModeCommand(cible, mode, args.size()) == false)
		return ;	
	
	std::map<char, std::string> modeArgs = IrcHelper::mapModesToArgs(args);
	_applyChannelModes(mode, cible, modeArgs);			//fonction qui traite les modes et dirige vers les bonnes fonctions	
}

/**
 * @brief Checks the mode arguments and executes the corresponding functions based on the mode.
 *
 * This function parses the mode string and executes the appropriate functions for each mode
 * based on the corresponding sign ('+' or '-'). It handles the following modes:
 * - 'i': Invite-only mode
 * - 't': Topic restriction mode
 * - 'k': Password mode
 * - 'o': Operator privilege mode
 * - 'l': Channel limit mode
 *
 * If an unknown mode is encountered, an error message is sent to the client.
 *
 * @param mode The mode string containing the modes to be applied.
 * @param channel The name of the channel on which the modes are to be applied.
 * @param modeArgs A map containing the arguments for the modes.
 */
void CommandHandler::_applyChannelModes(std::string &mode, std::string &channelName, std::map<char, std::string> &modeArgs)
{
	// une fois les elements parses, execute les fonctions correspondantes
	// a chaque mode en fonction du signe correspondant

	Channel* channel = _channels[channelName];

	if (mode.find('i') != std::string::npos)
	{
		_modeSign = mode[IrcHelper::findCharBeforeIndex(mode, '-', '+', mode.find('i'))];	//chope le bon signe a partir de l'indice ou se trouve ik
		_inviteOnly(channel, _modeSign);
	}
	if (mode.find('t') != std::string::npos)
	{
		_modeSign = mode[IrcHelper::findCharBeforeIndex(mode, '-', '+', mode.find('t'))];
		_topicRestriction(channel, _modeSign);
	}
	if (mode.find('k') != std::string::npos)
	{
		_modeSign = mode[IrcHelper::findCharBeforeIndex(mode, '-', '+', mode.find('k'))];
		_passwordMode(modeArgs['k'], channel, _modeSign, _client);
	}
	if (mode.find('o') != std::string::npos)
	{
		_modeSign = mode[IrcHelper::findCharBeforeIndex(mode, '-', '+', mode.find('o'))];
		int serverClientFd = _server.getClientByNickname(modeArgs.at('o'), NULL);
		int channelClientFd = channel->getChannelClientByNickname(modeArgs.at('o'), NULL);

		if (IrcHelper::clientExists(serverClientFd) == false)
			_client->sendMessage(MessageBuilder::ircNoSuchNick(_client->getNickname(), modeArgs.at('o')), NULL);
		else if (IrcHelper::clientExists(channelClientFd) == false)
			_client->sendMessage(MessageBuilder::ircNotInChannel(_client->getNickname(), channelName, modeArgs.at('o')), NULL);
		else
		{
			Client *newOp = _clients[_server.getClientByNickname(modeArgs.at('o'), NULL)];
			_operatorPrivilege(channel, _modeSign, newOp);
		}
	}
	if (mode.find('l') != std::string::npos)
	{
		_modeSign = mode[IrcHelper::findCharBeforeIndex(mode, '-', '+', mode.find('l'))];
		if (!_channelLimit(channel, _modeSign, modeArgs['l']))
			_client->sendMessage(MessageBuilder::ircInvalidModeParams(_client->getNickname(), channelName, "l", modeArgs['l']), NULL); //erreur si l arg est pas compose de digit
	}
	for (int i = 1; mode[i]; i++)
	{
		if (mode[i] != 'i' && mode[i] != 't' && mode[i] != 'k' && mode[i] != 'o' && mode[i] != 'l' && mode[i] != '-' && mode[i] != '+')
		{
			_client->sendMessage(MessageBuilder::ircUnknownMode(_client->getNickname(), mode[i]), NULL);
		}
	}
}

/**
 * @brief Checks if the given mode string is valid for the specified channel and the number of arguments.
 * 
 * This function verifies that the mode string is not empty, has at least two characters, and starts with either
 * '+' or '-'. It also ensures that the mode string contains valid mode characters ('i', 't', 'k', 'o', 'l') and
 * checks for duplicate mode characters. Finally, it verifies that the number of arguments matches the expected
 * number of arguments for the given mode string.
 * 
 * @param mode The mode string to be checked.
 * @param channel The channel for which the mode is being set.
 * @param nArgs The number of arguments provided for the mode.
 * 
 * @throws std::invalid_argument if the mode string is invalid or if the number of arguments does not match the expected number.
 */
void CommandHandler::_validateModeArguments(std::string &mode, const Channel *channel, unsigned int nArgs)
{
	// Check si le nombre d arguments et les arguments attendus sont les bons.
	if (mode.empty() || mode.size() < 2 || (mode[0] != '-' && mode[0] != '+'))
		throw std::invalid_argument((MessageBuilder::ircChannelModeIs(_client->getNickname(), channel->getName(), channel->getMode())));
	
	for (size_t i = 0; i < mode.size(); i++)
		if (mode[i] == 'i' || mode[i] == 't' || mode[i] == 'k' || mode[i] != 'o' || mode[i] != 'l')
			IrcHelper::assertNoDuplicate(mode, mode[i], i);
	
	if (nArgs != IrcHelper::getExpectedArgCount(mode))
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), MODE)); 
}

/**
 * @brief Handles simple commands and regular errors for the /mode command.
 * 
 * This function is called when the /mode command is sent. It performs an initial check for potential errors 
 * and parses the necessary elements before executing the command.
 * 
 * @param channel The channel name or user nickname.
 * @param mode The mode to be set or checked.
 * @param nArgs The number of arguments provided with the command.
 * @return true if the command is valid and can be processed further.
 * @return false if the command is invalid or has been handled within this function.
 * 
 * @throws std::invalid_argument if the client is not in the channel or is not an operator.
 */
bool CommandHandler::_validateModeCommand(std::string &channelName, std::string &mode, unsigned int nArgs)
{
	// fonction appellee lorsque la commande /mode est envoyee : 
	// fait un premier check d erreurs potentiolles et parse les bons elements avant de lancer checkargandexecute

	if ((channelName == _client->getNickname() && mode == "+i")
		|| (IrcHelper::isRightChannel(*_client, channelName, _channels, PRINT_ERROR) != channel_error::ALL_RIGHT))
		return false;

	Channel* channel = _channels[channelName];

	if (nArgs == 1)
	{
		_client->sendMessage((MessageBuilder::ircChannelModeIs(_client->getNickname(), channelName, channel->getMode())), NULL);	
		return false;
	}
	if (mode == "b")
	{
		_client->sendMessage(MessageBuilder::ircEndOfBannedList(_client->getNickname(), channelName), NULL);
		return false;	
	}

	int channelClientFd = channel->getChannelClientByNickname(_client->getNickname(), NULL);
	if (IrcHelper::clientExists(channelClientFd) == false)
		throw std::invalid_argument(MessageBuilder::ircCurrentNotInChannel(_client->getNickname(), channelName));
	if (_client->isOperator(channel) == false)
		throw std::invalid_argument(MessageBuilder::ircNotChanOperator(channelName));

	_validateModeArguments(mode, channel, nArgs);
	return true ;
}


// === MODE HANDLER ===

/**
 * @brief Handles the invite-only mode for a channel.
 *
 * This function sets or unsets the invite-only mode for the specified channel
 * based on the provided mode sign. If the mode sign is '+', the invite-only
 * mode is enabled. If the mode sign is '-', the invite-only mode is disabled.
 * It also sends a message to all clients in the channel indicating the change
 * in mode.
 *
 * @param channel Pointer to the Channel object for which the invite-only mode
 *                is to be set or unset.
 * @param modeSign Character indicating whether to set ('+') or unset ('-')
 *                 the invite-only mode.
 */
void CommandHandler::_inviteOnly(Channel *channel, char modeSign)
{
	std::string sign(1, modeSign);
	if (IrcHelper::noChangeToMake(modeSign, channel->getInvites()))
		return ;

	if (modeSign == '+' && !channel->getInvites())
		channel->setInvites(true);
	else
		channel->setInvites(false);

	_client->sendToAll(channel, MessageBuilder::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "i", ""), true);
}

/**
 * @brief Handles the topic restriction mode for a given channel.
 *
 * This function sets or unsets the topic restriction mode ('t') for the specified channel
 * based on the provided mode sign. If the mode sign is '+', the topic restriction is enabled.
 * If the mode sign is '-', the topic restriction is disabled. The function also sends a message
 * to all clients in the channel to notify them of the mode change.
 *
 * @param channel Pointer to the Channel object for which the topic restriction mode is being set.
 * @param modeSign Character representing the mode sign ('+' to enable, '-' to disable).
 */
void CommandHandler::_topicRestriction(Channel *channel, char modeSign)
{
	std::string sign(1, modeSign);
	if (IrcHelper::noChangeToMake(modeSign, channel->getRightsTopic()))
		return ;

	if (modeSign == '+' && !channel->getRightsTopic())
		channel->setRightsTopic(true);
	else
		channel->setRightsTopic(false);

	_client->sendToAll(channel, MessageBuilder::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "t", ""), true);
}

/**
 * @brief Handles the password mode change for a channel.
 *
 * This function sets or removes the password for a given channel based on the mode sign.
 * If the mode sign is '+', it sets the password to the provided argument.
 * If the mode sign is '-', it removes the current password.
 *
 * @param args The password to be set or an empty string to remove the password.
 * @param channel The channel for which the password mode is being changed.
 * @param modeSign The mode sign indicating whether to set ('+') or remove ('-') the password.
 * @param client The client requesting the password mode change.
 */
void CommandHandler::_passwordMode(std::string args, Channel *channel, char modeSign, Client *client)
{
	std::string sign(1, modeSign);
	if (!IrcHelper::isValidPassword(args, false) && modeSign == '+')
	{
		_client->sendMessage(MessageBuilder::ircInvalidPasswordFormat(client->getNickname(), channel->getName()), NULL);
		_client->sendMessage(MessageBuilder::ircCurrentNotInChannel(_client->getNickname(), channel->getName()), NULL);
		return ;
	}

	if (modeSign == '-' && channel->getPassword().empty())
		return ;

	if (modeSign == '+')
		channel->setPassword(args);
	else
		channel->setPassword("");

	_client->sendToAll(channel, MessageBuilder::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "k", ""), true);
}

/**
 * @brief Handles the operator privilege mode change for a given channel.
 *
 * This function adds or removes an operator privilege for a client in a specified channel
 * based on the mode sign provided. It also sends a notification to all clients in the channel
 * about the mode change.
 *
 * @param channel Pointer to the Channel object where the mode change is to be applied.
 * @param modeSign Character indicating the mode change ('+' to add operator, '-' to remove operator).
 * @param newOp Pointer to the Client object representing the client whose operator status is to be changed.
 */
void CommandHandler::_operatorPrivilege(Channel *channel, char modeSign, Client *newOp)
{
	std::string sign(1, modeSign);
	if (IrcHelper::noChangeToMake(modeSign, channel->isOperator(newOp)))
		return ;

	if (modeSign == '+' && !channel->isOperator(newOp))
		channel->addOperator(newOp);
	else
		channel->removeOperator(newOp);
		
	_client->sendToAll(channel, MessageBuilder::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "o", newOp->getNickname()), true);
}

/**
 * @brief Handles the channel limit mode change.
 *
 * This function processes the mode change for channel limits. It sets or removes
 * the limit on the number of users that can join the channel based on the mode sign
 * and arguments provided.
 *
 * @param channel Pointer to the Channel object where the mode change is applied.
 * @param modeSign Character indicating the mode change ('+' to set limit, '-' to remove limit).
 * @param args String containing the limit value when setting the limit.
 * @return true if the mode change was successfully applied, false otherwise.
 */
bool CommandHandler::_channelLimit(Channel *channel, char modeSign, std::string args)
{
	std::string sign(1, modeSign);
	if (modeSign == '-' && channel->getLimits() == -1)
		return true;
	
	if (modeSign == '+')
	{
		if (args.size() > 0)
		{
			if (!IrcHelper::isValidLimit(args))
				return (false);
			if (std::atol(args.c_str()) == channel->getLimits())
				return (true);
			channel->setLimits(std::atol(args.c_str()));
		}
	}
	else
		channel->setLimits(-1);
		
	_client->sendToAll(channel, MessageBuilder::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "l", args), true);
	return true;
}