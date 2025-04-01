/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler_Mode.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 13:03:01 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommandHandler.hpp"

// === OTHER CLASSES ===
#include "Utils.hpp"
#include "IrcHelper.hpp"
#include "MessageBuilder.hpp"

// === NAMESPACES ===
#include "irc_config.hpp"
#include "commands.hpp"

using namespace commands;
using namespace error_display;

// =========================================================================================

// === MODE ===

// ========================================= PRIVATE =======================================

// === MODE PARSER ===

/**
 * @brief Handles the mode change command for the IRC server.
 *
 * This function processes the mode change command by parsing the input,
 * validating the command, and applying the appropriate channel modes.
 *
 * @details
 * - Extracts the target and mode arguments from the input.
 * - Validates the mode command to ensure it is properly formatted.
 * - Maps the mode arguments to their corresponding values.
 * - Applies the channel modes to the specified target.
 *
 * @note This function assumes that `_itInput` points to the current input
 *       command and that `splitter::WORD` is used to tokenize the input.
 *
 * @return void
 */
void CommandHandler::_handleMode()
{
	std::string mode;
	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::WORD);
	std::string target = *args.begin();
	if (args.size() > 1)
		_mode = *++args.begin();
	if (_validateModeCommand(target, args.size()) == false)
		return;	
	
	_modeArgs = IrcHelper::mapModesToArgs(args);
	_applyChangeMode(target);
}

/**
 * @brief Validates the MODE command for a given channel and client.
 * 
 * This function checks if the MODE command is valid based on the provided
 * channel name, the number of arguments, and the current state of the client
 * and channel. It ensures that the client has the necessary permissions and
 * that the command is properly formatted.
 * 
 * @param channelName The name of the channel for which the MODE command is issued.
 * @param nArgs The number of arguments provided with the MODE command.
 * @return true If the MODE command is valid and can proceed.
 * @return false If the MODE command is invalid or does not require further processing.
 * 
 * @throws std::invalid_argument If the client is not in the channel or is not an operator.
 * 
 * @details
 * - If the channel name matches the client's nickname and the mode is "+i", or if the
 *   channel validation fails, the function returns false.
 * - If only one argument is provided, the function sends the current channel modes
 *   to the client and returns false.
 * - If the mode is "b", the function sends the end of the banned list message to the
 *   client and returns false.
 * - If the client is not in the channel or is not an operator, an exception is thrown.
 * - The function validates the mode arguments if all checks pass.
 */
bool CommandHandler::_validateModeCommand(const std::string& channelName, unsigned int nArgs)
{
	if ((channelName == _client->getNickname() && _mode == "+i")
		|| (IrcHelper::isRightChannel(*_client, channelName, _channels, PRINT_ERROR) != channel_error::ALL_RIGHT))
		return false;

	Channel* channel = _channels[channelName];
	if (nArgs == 1 || _mode == "b")
	{
		std::string modeMsg = MessageBuilder::ircChannelModeIs(_client->getNickname(), channelName, channel->getModes());
		std::string banMsg = MessageBuilder::ircEndOfBannedList(_client->getNickname(), channelName);
		nArgs == 1 ? _client->sendMessage(modeMsg, NULL) : _client->sendMessage(banMsg, NULL);
		return false;
	}

	int channelClientFd = channel->getChannelClientByNickname(_client->getNickname(), NULL);
	if (IrcHelper::clientExists(channelClientFd) == false)
		throw std::invalid_argument(MessageBuilder::ircCurrentNotInChannel(_client->getNickname(), channelName));
	if (_client->isOperator(channel) == false)
		throw std::invalid_argument(MessageBuilder::ircNotChanOperator(channelName));

	_validateModeArguments(channel, nArgs);
	return true ;
}

/**
 * @brief Validates the mode arguments for a channel command.
 * 
 * This function ensures that the mode string provided is valid and adheres to
 * the expected format and rules. It checks for proper prefixes, duplicate mode
 * flags, and the correct number of arguments based on the mode string.
 * 
 * @param channel A pointer to the Channel object for which the mode is being set.
 * @param nArgs The number of arguments provided for the mode command.
 * 
 * @throws std::invalid_argument If the mode string is invalid, contains duplicates,
 *         or the number of arguments does not match the expected count.
 */
void CommandHandler::_validateModeArguments(const Channel *channel, unsigned int nArgs)
{
	if (_mode.empty() || _mode.size() < 2 || (_mode[0] != '-' && _mode[0] != '+'))
		throw std::invalid_argument((MessageBuilder::ircChannelModeIs(_client->getNickname(), channel->getName(), channel->getModes())));
	
	for (size_t i = 0; i < _mode.size(); i++)
		if (_mode[i] == 'i' || _mode[i] == 't' || _mode[i] == 'k' || _mode[i] != 'o' || _mode[i] != 'l')
			IrcHelper::assertNoDuplicate(_mode, _mode[i], i);
	
	if (nArgs != IrcHelper::getExpectedArgCount(_mode))
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), MODE)); 
}

/**
 * @brief Applies mode changes to a specified channel based on the provided mode string.
 *
 * This function iterates through the mode string (_mode) and applies the corresponding
 * mode changes to the specified channel. It uses a map of mode characters to handler
 * functions to determine the appropriate action for each mode character.
 *
 * @param channelName The name of the channel to which the mode changes will be applied.
 *
 * The function performs the following:
 * - Identifies the mode character and determines whether it is valid.
 * - Calls the corresponding handler function for the mode character if it exists.
 * - Sends an error message to the client if an unknown mode character is encountered.
 *
 * Mode characters and their corresponding handlers:
 * - 'i': Sets the channel to invite-only mode.
 * - 't': Restricts topic changes to channel operators.
 * - 'k': Sets a password for the channel.
 * - 'o': Grants operator privileges to a user.
 * - 'l': Sets a user limit for the channel.
 *
 * If a mode character is preceded by '+' or '-', the _modeSign variable is updated
 * to reflect the current mode operation (add or remove).
 *
 * Error Handling:
 * - If an unknown mode character is encountered, an error message is sent to the client
 *   using the MessageBuilder::ircUnknownMode function.
 */
void CommandHandler::_applyChangeMode(const std::string& channelName)
{
	Channel* channel = _channels[channelName];

	typedef void (CommandHandler::*ModeHandler)(Channel*);
	std::map<char, ModeHandler> modeHandlers;
	
	modeHandlers['i'] = &CommandHandler::_setInviteOnly;
	modeHandlers['t'] = &CommandHandler::_setTopicRestriction;
	modeHandlers['k'] = &CommandHandler::_setPasswordMode;
	modeHandlers['o'] = &CommandHandler::_setOperatorPrivilegeWrapper;
	modeHandlers['l'] = &CommandHandler::_setChannelLimitWrapper;

	for (std::string::size_type i = 0; i < _mode.size(); ++i)
	{
		char modeChar = _mode[i];
		if (modeHandlers.find(modeChar) != modeHandlers.end())
		{
			_modeSign = _mode[IrcHelper::findCharBeforeIndex(_mode, '-', '+', _mode.find(modeChar))];
			(this->*modeHandlers[modeChar])(channel);
		}
		else if (modeChar != '-' && modeChar != '+')
			_client->sendMessage(MessageBuilder::ircUnknownMode(_client->getNickname(), modeChar), NULL);
	}
}

/**
 * @brief Grants operator privileges to a client in a specific channel.
 *
 * This function is a wrapper that handles the process of setting operator
 * privileges for a client in a given channel. It validates the existence of
 * the client both on the server and in the channel before delegating the
 * operation to the internal `_setOperatorPrivilege` method.
 *
 * @param channel A pointer to the Channel object where the operator privilege
 *                is to be set.
 *
 * @details
 * - The function checks if the client specified in `_modeArgs` exists on the
 *   server and in the channel.
 * - If the client does not exist on the server, an error message is sent to
 *   the requesting client indicating "No such nick".
 * - If the client exists on the server but not in the channel, an error
 *   message is sent to the requesting client indicating "Not in channel".
 * - If the client exists in both the server and the channel, the function
 *   retrieves the client object and delegates the privilege assignment to
 *   `_setOperatorPrivilege`.
 */
void CommandHandler::_setOperatorPrivilegeWrapper(Channel *channel)
{
	int serverClientFd = _server.getClientByNickname(_modeArgs.at('o'), NULL);
	int channelClientFd = channel->getChannelClientByNickname(_modeArgs.at('o'), NULL);

	if (!IrcHelper::clientExists(serverClientFd) || !IrcHelper::clientExists(channelClientFd))
	{
		std::string msgNoNick = MessageBuilder::ircNoSuchNick(_client->getNickname(), _modeArgs.at('o'));
		std::string msgNotInChan = MessageBuilder::ircNotInChannel(_client->getNickname(), channel->getName(), _modeArgs.at('o'));
		!IrcHelper::clientExists(serverClientFd) ? _client->sendMessage(msgNoNick, NULL) : _client->sendMessage(msgNotInChan, NULL);
		return;
	}
	Client *newOp = _clients[serverClientFd];
	_setOperatorPrivilege(channel, newOp);
}

/**
 * @brief Wrapper function to set the channel limit for a given channel.
 *
 * This function attempts to set a limit on the specified channel by calling
 * the internal `_setChannelLimit` method. If the operation fails, it sends
 * an error message to the client indicating invalid mode parameters.
 *
 * @param channel Pointer to the Channel object for which the limit is to be set.
 */
void CommandHandler::_setChannelLimitWrapper(Channel *channel)
{
	if (!_setChannelLimit(channel))
		_client->sendMessage(MessageBuilder::ircInvalidModeParams(_client->getNickname(), channel->getName(), "l", _modeArgs['l']), NULL);
}

// =========================================================================================

// === MODE HANDLER ===

/**
 * @brief Sets the invite-only mode for a given channel based on the current mode sign.
 *
 * This function checks if there is a need to change the invite-only mode of the channel.
 * If the mode sign indicates a change ('+' to enable or '-' to disable) and the current
 * state of the channel does not match the desired state, the invite-only mode is updated.
 * After the change, a notification is sent to all clients in the channel about the mode update.
 *
 * @param channel A pointer to the Channel object whose invite-only mode is to be modified.
 */
void CommandHandler::_setInviteOnly(Channel *channel)
{
	if (IrcHelper::noChangeToMake(_modeSign, channel->isInviteOnly()))
		return;
	_modeSign == '+' && !channel->isInviteOnly() ? channel->setInviteOnly(true) : channel->setInviteOnly(false);

	std::string sign(1, _modeSign);
	_client->sendToAll(channel, MessageBuilder::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "i", ""), true);
}

/**
 * @brief Updates the topic restriction mode of a channel.
 *
 * This function modifies the topic restriction setting of the specified channel
 * based on the current mode sign (`_modeSign`). If the mode sign is '+' and the
 * channel does not already have the topic restriction enabled, the restriction
 * is enabled. If the mode sign is '-' and the channel has the topic restriction
 * enabled, the restriction is disabled. If no change is required, the function
 * returns early.
 *
 * After updating the topic restriction, the function notifies all clients in
 * the channel about the mode change.
 *
 * @param channel A pointer to the Channel object whose topic restriction mode
 *                is to be updated.
 */
void CommandHandler::_setTopicRestriction(Channel *channel)
{
	if (IrcHelper::noChangeToMake(_modeSign, channel->isSettableTopic()))
		return;
	_modeSign == '+' && !channel->isSettableTopic() ? channel->setSettableTopic(true) : channel->setSettableTopic(false);

	std::string sign(1, _modeSign);
	_client->sendToAll(channel, MessageBuilder::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "t", ""), true);
}

/**
 * @brief Sets the password mode for a given channel based on the current mode sign.
 *
 * This function checks if the password format is valid and sets or clears the
 * password for the specified channel based on the mode sign ('+' to set, '-' to clear).
 * If the password format is invalid, an error message is sent to the client.
 *
 * @param channel A pointer to the Channel object whose password mode is to be modified.
 */
void CommandHandler::_setPasswordMode(Channel *channel)
{
	std::string password = _modeArgs['k'];
	if (!IrcHelper::isValidPassword(password, false) && _modeSign == '+')
	{
		_client->sendMessage(MessageBuilder::ircInvalidPasswordFormat(_client->getNickname(), channel->getName()), NULL);
		_client->sendMessage(MessageBuilder::ircCurrentNotInChannel(_client->getNickname(), channel->getName()), NULL);
		return;
	}
	if (_modeSign == '-' && channel->getPassword().empty())
		return;
	_modeSign == '+' ? channel->setPassword(password) : channel->setPassword("");

	std::string sign(1, _modeSign);
	_client->sendToAll(channel, MessageBuilder::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "k", ""), true);
}

/**
 * @brief Adjusts the operator privilege of a client in a channel.
 *
 * This function modifies the operator status of a client in the specified channel
 * based on the current mode sign (`+` or `-`). If the mode sign is `+`, the client
 * is granted operator privileges. If the mode sign is `-`, the client is stripped
 * of operator privileges. If no change is needed (e.g., the client already has or
 * does not have operator privileges as required), the function returns early.
 *
 * @param channel A pointer to the Channel object where the operator privilege
 *                is to be modified.
 * @param newOp A pointer to the Client object whose operator privilege is to
 *              be adjusted.
 */
void CommandHandler::_setOperatorPrivilege(Channel *channel, Client *newOp)
{
	if (IrcHelper::noChangeToMake(_modeSign, channel->isOperator(newOp)))
		return;
	_modeSign == '+' && !channel->isOperator(newOp) ? channel->addOperator(newOp) : channel->removeOperator(newOp);

	std::string sign(1, _modeSign);		
	_client->sendToAll(channel, MessageBuilder::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "o", newOp->getNickname()), true);
}

/**
 * @brief Handles the setting or unsetting of the client limit for a channel.
 * 
 * This function processes the mode change for the 'l' (limit) flag on a channel.
 * It either sets a new client limit or removes the limit based on the mode sign.
 * 
 * @param channel Pointer to the Channel object on which the limit is being set or removed.
 * @return true if the operation was successful, false otherwise.
 * 
 * The function performs the following steps:
 * - If the mode sign is '-' and the current client limit is already unset (-1), it returns true.
 * - If the mode sign is '+', it validates the new limit and sets it if valid.
 * - If the mode sign is '-', it removes the client limit by setting it to -1.
 * - Sends a notification to all clients in the channel about the mode change.
 * 
 * @note The function uses `_modeArgs` to retrieve the new limit value and `_modeSign` to determine
 *       whether the mode is being added ('+') or removed ('-').
 * @note The function relies on `IrcHelper::isValidLimit` to validate the new limit string.
 * @note The function sends a message to all clients in the channel using `MessageBuilder::ircOpeChangedMode`.
 */
bool CommandHandler::_setChannelLimit(Channel *channel)
{
	std::string newLimitStr = _modeArgs['l'];
	int newLimit = std::atol(newLimitStr.c_str());
	if (_modeSign == '-' && channel->getClientsLimit() == -1)
		return true;
	
	if (_modeSign == '+')
	{
		if (newLimitStr.size() > 0)
		{
			if (!IrcHelper::isValidLimit(newLimitStr))
				return false;
			if (newLimit == channel->getClientsLimit())
				return true;
			channel->setClientsLimit(newLimit);
		}
	}
	else
		channel->setClientsLimit(-1);
		
	std::string sign(1, _modeSign);
	_client->sendToAll(channel, MessageBuilder::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "l", newLimitStr), true);
	return true;
}