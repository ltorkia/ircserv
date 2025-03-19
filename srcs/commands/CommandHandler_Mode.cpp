#include "../../incs/classes/CommandHandler.hpp"
#include "../../incs/classes/Channel.hpp"

// === NAMESPACES ===
using namespace commands;
using namespace channel_error;
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
	std::string cible = *args.begin();					//channel cible
	if (args.size() > 1)
		mode = *++args.begin();							//mode
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

		if (IrcHelper::clientExists(serverClientFd) == false)	//erreur si nom de l'operateur inconnu sur le serveur
			_client->sendMessage(MessageHandler::ircNoSuchNick(_client->getNickname(), modeArgs.at('o')), NULL);
		else if (IrcHelper::clientExists(channelClientFd) == false)	//erreur si nom de l'operateur inconnu sur le channel
			_client->sendMessage(MessageHandler::ircNotInChannel(_client->getNickname(), channelName, modeArgs.at('o')), NULL);
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
			_client->sendMessage(MessageHandler::ircInvalidModeParams(_client->getNickname(), channelName, "l", modeArgs['l'], "need only digital char"), NULL); //erreur si l arg est pas compose de digit
	}
	for (int i = 1; mode[i]; i++)
	{
		if (mode[i] != 'i' && mode[i] != 't' && mode[i] != 'k' && mode[i] != 'o' && mode[i] != 'l' && mode[i] != '-' && mode[i] != '+')
		{
			_client->sendMessage(MessageHandler::ircUnknownMode(_client->getNickname(), mode[i]), NULL); //erreur a envoye si des modes sont inconnus une fois les modes connus executes
			return;
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
		throw std::invalid_argument((MessageHandler::ircChannelModeIs(_client->getNickname(), channel->getName(), channel->getMode())));
	
	for (size_t i = 0; i < mode.size(); i++)
		if (mode[i] == 'i' || mode[i] == 't' || mode[i] == 'k' || mode[i] != 'o' || mode[i] != 'l')
			IrcHelper::assertNoDuplicate(mode, mode[i], i);
	
	if (nArgs != IrcHelper::getExpectedArgCount(mode))			//minimum 2 elements attendus et max en fonction du nmbre d args attendus
		throw std::invalid_argument(MessageHandler::ircNeedMoreParams(_client->getNickname(), MODE)); 
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
		|| (IrcHelper::isRightChannel(*_client, channelName, _channels, PRINT_ERROR) != ALL_RIGHT))
		return false;

	Channel* channel = _channels[channelName];

	if (nArgs == 1)
	{
		_client->sendMessage((MessageHandler::ircChannelModeIs(_client->getNickname(), channelName, channel->getMode())), NULL);	
		return false;
	}
	if (mode == "b")
	{
		_client->sendMessage(MessageHandler::ircEndOfBannedList(_client->getNickname(), channelName), NULL);
		return false;	
	}

	int channelClientFd = channel->getChannelClientByNickname(_client->getNickname(), NULL);
	if (IrcHelper::clientExists(channelClientFd) == false)
		throw std::invalid_argument(MessageHandler::ircCurrentNotInChannel(_client->getNickname(), channelName));
	if (_client->isOperator(channel) == false)
		throw std::invalid_argument(MessageHandler::ircNotChanOperator(channelName));

	_validateModeArguments(mode, channel, nArgs);
	return true ;
}


// === MODE HANDLER ===

// Gestion du mod 'i', se refere au booleen dans Channel.hpp
void CommandHandler::_inviteOnly(Channel *channel, char modeSign)
{
	std::string sign(1, modeSign);
	if (IrcHelper::noChangeToMake(modeSign, channel->getInvites()))
		return ;

	if (modeSign == '+' && !channel->getInvites())
		channel->setInvites(true);
	else
		channel->setInvites(false);

	_client->sendToAll(channel, MessageHandler::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "i", ""), true);
}

// Gestion du mod 't', se refere au booleen dans Channel.hpp
void CommandHandler::_topicRestriction(Channel *channel, char modeSign)
{
	std::string sign(1, modeSign);
	if (IrcHelper::noChangeToMake(modeSign, channel->getRightsTopic()))
		return ;

	if (modeSign == '+' && !channel->getRightsTopic())
		channel->setRightsTopic(true);
	else
		channel->setRightsTopic(false);

	_client->sendToAll(channel, MessageHandler::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "t", ""), true);
}

// Gestion du mod 'k', se refere au booleen dans Channel.hpp
void CommandHandler::_passwordMode(std::string args, Channel *channel, char modeSign, Client *client)
{
	std::string sign(1, modeSign);
	if (!IrcHelper::isValidPassword(args, false) && modeSign == '+')
	{
		_client->sendMessage(MessageHandler::ircInvalidPasswordFormat(client->getNickname(), channel->getName()), NULL);
		_client->sendMessage(MessageHandler::ircCurrentNotInChannel(_client->getNickname(), channel->getName()), NULL);
		return ;
	}

	if (modeSign == '-' && channel->getPassword().empty())
		return ;

	if (modeSign == '+')
		channel->setPassword(args);
	else
		channel->setPassword("");

	_client->sendToAll(channel, MessageHandler::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "k", ""), true);
}
// Gestion du mod 'o', se refere au booleen dans Channel.hpp et Client.hpp
void CommandHandler::_operatorPrivilege(Channel *channel, char modeSign, Client *newOp)
{
	std::string sign(1, modeSign);
	if (IrcHelper::noChangeToMake(modeSign, channel->isOperator(newOp)))
		return ;

	if (modeSign == '+' && !channel->isOperator(newOp))
		channel->addOperator(newOp);
	else
		channel->removeOperator(newOp);
		
	_client->sendToAll(channel, MessageHandler::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "o", newOp->getNickname()), true);
}

// Gestion du mod 'l', se refere au booleen dans Channel.hpp et Client.hpp
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
		
	_client->sendToAll(channel, MessageHandler::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "l", args), true);
	return true;
}