#include "../../../incs/server/CommandHandler.hpp"

// === OTHER CLASSES ===
#include "../../../incs/utils/Utils.hpp"
#include "../../../incs/utils/IrcHelper.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/commands.hpp"
#include "../../../incs/config/server_messages.hpp"
#include "../../../incs/config/colors.hpp"
#include "../../../incs/config/bot_config.hpp"

using namespace commands;
using namespace auth_cmd;
using namespace server_messages;
using namespace colors;
using namespace bot_config;

// =========================================================================================

// === COMMAND HANDLER ===

// ========================================= PRIVATE =======================================

// === AUTHENTICATE COMMANDS ===

/**
 * @brief Authenticates and processes an IRC command.
 *
 * This function checks if the current command is valid and if the client is authenticated.
 * If the command is not valid or the client is not authenticated, it sends appropriate error messages.
 * If the command is valid, it executes the corresponding function from the command map.
 * If the client becomes authenticated after processing the command, it sends a greeting message and a registration prompt.
 *
 * The function performs the following steps:
 * 1. Retrieves the current command and determines the next command to send.
 * 2. Checks if the command is valid and if the client is allowed to execute it.
 * 3. Sends error messages if the command is invalid or the client is not authenticated.
 * 4. Executes the corresponding function for the command if it is valid.
 * 5. Sends a command prompt if the client is not fully authenticated.
 * 6. Authenticates the client and sends a greeting message if the client becomes fully authenticated.
 */
void CommandHandler::_authenticate()
{
	std::string cmd = *_itInput;
	std::string command_to_send = IrcHelper::commandToSend(*_client);
	int toDo = IrcHelper::getCommand(*_client);
	std::map<std::string, void (CommandHandler::*)()>::iterator itFunction = _fctMap.find(*_itInput);

	if (itFunction == _fctMap.end() || IrcHelper::isCommandIgnored(cmd, true)
		|| (cmd == NICK && toDo != NICK_CMD) || (cmd == USER && toDo != USER_CMD))
	{
		if (_client->isIdentified() == true)
		{
			_preRegister(cmd, toDo);
			return;
		}
		_client->sendMessage(MessageBuilder::ircNotRegistered(), NULL);
		if (_client->isIdentified() == false)
			_client->sendMessage(MessageBuilder::ircCommandPrompt(command_to_send, cmd), NULL);
		return;
	}
	
	_itInput++;
	(this->*itFunction->second)();

	toDo = IrcHelper::getCommand(*_client);
	if (toDo < CMD_ALL_SET && IrcHelper::isCommandIgnored(cmd, false) && !_client->isIdentified())
	{
		command_to_send = IrcHelper::commandToSend(*_client);
		_client->sendMessage(MessageBuilder::ircCommandPrompt(command_to_send, ""), NULL);
	}

	if (toDo == CMD_ALL_SET && _client->isAuthenticated() == false)
	{
		_client->setUsermask();
		_client->authenticate();
		_server.greetClient(_client);
		_client->sendMessage(MessageBuilder::ircNoticeMsg(_client->getNickname(), PROMPT_ONCE_REGISTERED, IRC_COLOR_INFO), NULL);
		std::cout << MessageBuilder::msgClientConnected(_client->getClientIp(), _client->getClientPort(), _clientFd, _client->getNickname()) << std::endl;
	}
}

/**
 * @brief Handles pre-registration commands for the client.
 *
 * This function processes commands before the client is fully registered.
 * It checks if there are additional elements in the command vector and
 * sets the appropriate identification command for the client based on
 * the provided command and action to be performed.
 *
 * @param cmd The command to be processed (e.g., "NICK", "USER").
 * @param toDo An integer indicating the action to be performed.
 *              - If cmd is "NICK" and toDo is not 1, sets the client's nickname command.
 *              - If cmd is "USER" and toDo is not 2, sets the client's username command.
 */
void CommandHandler::_preRegister(const std::string& cmd, int toDo) {
	if (_vectorInput.begin() + 1 != _vectorInput.end())
	{
		std::vector<std::string> identCmd(_vectorInput.begin() + 1, _vectorInput.end());
		if (cmd == "NICK" && toDo != NICK_CMD)
			_client->setIdentNickCmd(identCmd);
		if (cmd == "USER" && toDo != USER_CMD)
			_client->setIdentUsernameCmd(identCmd);
	}
}

/**
 * @brief Checks if the provided password is correct and updates the client's password validity status.
 * 
 * This function performs the following checks:
 * 1. If the iterator `_itInput` is at the end of `_vectorInput`, it throws an `std::invalid_argument` indicating that more parameters are needed.
 * 2. If the client has already provided a valid server password, it throws an `std::invalid_argument` indicating that the client is already registered.
 * 3. Extracts the password from the current iterator position.
 * 4. Compares the extracted password with the server's password. If they do not match, it throws an `std::invalid_argument` indicating that the password is incorrect.
 * 5. If the password is correct, it sends a success message to the client and sets the client's server password validity to true.
 * 
 * @throws std::invalid_argument if more parameters are needed, the client is already registered, or the password is incorrect.
 */
void CommandHandler::_isRightPassword()
{
	if (Utils::isEmptyOrInvalid(_itInput, _vectorInput))
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), PASS));
	if (_client->gotValidServPassword() == true)
		throw std::invalid_argument(MessageBuilder::ircAlreadyRegistered(_client->getNickname()));

	std::string password = *_itInput;
	const std::string& servPassword = _server.getServerPassword();

	if (password != servPassword)				
		throw std::invalid_argument(MessageBuilder::ircPasswordIncorrect());		

	_client->sendMessage(MessageBuilder::ircNoticeMsg(SERVER_PASSWORD_FOUND, IRC_COLOR_SUCCESS), NULL);
	_client->setServPasswordValidity(true);

	// Si le client a déjà donné un nickname via identification irssi, on le set directement
	std::vector<std::string> identNickCmd = _client->getIdentNickCmd();
	if (!identNickCmd.empty())
		_setNicknameClient();
}

/**
 * @brief Sets the nickname for the client.
 * 
 * This function handles the process of setting a nickname for the client. It performs several checks:
 * - Ensures the nickname command is not empty.
 * - Validates the nickname format.
 * - Checks if the nickname is already taken by another client.
 * 
 * If the nickname is valid and not taken, it sets the nickname for the client. If this is the first time the nickname is being set,
 * it sends a confirmation message to the client. If the client has already provided a username via identification, it sets the username directly.
 * 
 * If the nickname is being changed, it broadcasts the change to all clients.
 * 
 * @throws std::invalid_argument if the nickname is invalid, contains forbidden characters, or is already taken.
 */
void CommandHandler::_setNicknameClient(void)
{
	// Si le client a déjà donné un nickname via identification irssi, on le récupère
	std::vector<std::string> identNickCmd = _client->getIdentNickCmd();
	if (!identNickCmd.empty())
	{
		_itInput = identNickCmd.begin();
		_client->setIdentNickCmd(std::vector<std::string>());
	}

	// On stocke l'ancien nickname s'il existe
	std::string oldNickname = _client->getNickname();

	// Si le client n'a pas de nickname, on prend "*" par défaut pour les messages d'erreur
	std::string nickname = !oldNickname.empty() ? oldNickname : "*";

	// Check si argument existe
	if (Utils::isEmptyOrInvalid(_itInput, _vectorInput))
		throw std::invalid_argument(MessageBuilder::ircNoNicknameGiven(nickname));

	std::string enteredNickname = *_itInput;

	// On check s'il y a des caractères interdits
	if (IrcHelper::isValidName(enteredNickname, name_type::NICKNAME) == false)
		throw std::invalid_argument(MessageBuilder::ircErroneusNickname(nickname, enteredNickname));
	
	// On check si le nickname est déjà pris
	if (_server.getClientByNickname(enteredNickname, _client) != -1)
	{
		if (_client->isIdentified())
			_client->sendMessage(MessageBuilder::ircChangingNickname(enteredNickname), NULL);
		throw std::invalid_argument(MessageBuilder::ircNicknameTaken(nickname, enteredNickname));
	}

	// Si tout est ok, on set le nickname et on le stocke
	_client->setNickname(enteredNickname);
	std::string newNickname = _client->getNickname();

	// Affichage d'un message de confirmation au client si c'est le premier set du nickname
	if (oldNickname.empty())
	{
		_client->sendMessage(MessageBuilder::ircFirstNicknameSet(newNickname), NULL);
		_client->sendMessage(MessageBuilder::ircNicknameSet("", newNickname), NULL);

		// Si le client a déjà donné un username via identification irssi, on le set directement
		std::vector<std::string> identUserCmd = _client->getIdentUsernameCmd();
		if (!identUserCmd.empty())
			_setUsernameClient();
		return;
	}

	// Sinon, c'est un changement de nickname:
	// on broadcast à tous les clients pour valider le changement
	if ((!oldNickname.empty() && oldNickname != newNickname))
		_server.broadcastToClients(MessageBuilder::ircNicknameSet(oldNickname, newNickname));
}

/**
 * @brief Sets the username, hostname, and real name for a client.
 * 
 * This function processes the USER command from an IRC client, setting the 
 * username, hostname, and real name based on the provided arguments. It 
 * performs various checks to ensure the validity of the input and throws 
 * exceptions if any issues are found.
 * 
 * @throws std::invalid_argument if the USER command is missing parameters, 
 *         if the username is already set, or if any of the provided arguments 
 *         are invalid.
 */
void CommandHandler::_setUsernameClient(void)
{
	// Si le client a déjà donné un username via identification irssi, on le récupère
	std::vector<std::string> identUserCmd = _client->getIdentUsernameCmd();
	if (!identUserCmd.empty())
	{
		_itInput = identUserCmd.begin();
		_client->setIdentUsernameCmd(std::vector<std::string>());
	}

	// Check si argument existe ou si le username a déjà été set
	if (Utils::isEmptyOrInvalid(_itInput, _vectorInput))
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), USER));
	if (!(_client->getUsername().empty()))
		throw std::invalid_argument(MessageBuilder::ircAlreadyRegistered(_client->getUsername()));
	
	// Récupération des arguments
	std::vector<std::string> args = Utils::getTokens(*_itInput, splitter::WORD);
	if (args.size() < 4)
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), USER));
	
	std::vector<std::string>::iterator itArg = args.begin();

	// Récupération et validation du username, hostname et realname
	_usernameSettings(itArg);
	_hostnameSettings(itArg);
	_realNameSettings(itArg, args);

	// Envoi d'un message de confirmation au client
	_client->sendMessage(MessageBuilder::ircUsernameSet(_client->getUsername()), NULL);
}

/**
 * @brief Handles the setting of the username for a client.
 *
 * This function takes a const iterator to a vector of strings, extracts the username,
 * validates it, and sets it for the client. If the username is invalid, an exception
 * is thrown. The username is always prefixed with a tilde (~) and truncated to a
 * maximum of 10 characters if it is too long.
 *
 * @param itArg Const iterator to the vector of strings containing the username.
 * @throws std::invalid_argument if the username is invalid.
 */
void CommandHandler::_usernameSettings(const std::vector<std::string>::iterator& itArg)
{
	std::string username = *itArg;
	if (IrcHelper::isValidName(username, name_type::USERNAME) == false)
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), USER));

	// La machine n'étant pas identifiée par le ident protocol (non géré sur ce serveur),
	// on ajoute toujours ~ devant le username.
	// S'il est trop long, on le coupe après les 10 premiers caractères
	_client->setUsername(IrcHelper::formatUsername(username));
}

/**
 * @brief Handles the hostname settings for a client.
 *
 * This function processes the hostname argument provided by the client and sets
 * the client's hostname accordingly. If the hostname is invalid, it throws an
 * exception. If the hostname argument is "0", it replaces it with the client's
 * nickname. Additionally, if the client's IP is unknown, it sets the client's
 * IP to the provided hostname.
 *
 * @param itArg Iterator pointing to the current argument in the list of arguments.
 * @throws std::invalid_argument if the hostname is invalid.
 */
void CommandHandler::_hostnameSettings(std::vector<std::string>::iterator& itArg)
{
	std::string hostname = *++itArg;
	if (IrcHelper::isValidName(hostname, name_type::HOSTNAME) == false)
	{
		_client->setUsername("");
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), USER));
	}

	// Si hostname arg = "0", on le remplace par le nickname
	if (hostname == "0")
		hostname = _client->getNickname();
	_client->setHostname(hostname);

	// Si l'IP du client est inconnue, on remplace par le hostname tout juste fourni
	if (_client->getClientIp() == server::UNKNOWN_IP)
		_client->setClientIp(hostname);

	// on saute le champ inutilisé (*, servername ou client IP -> déjà enregistré)
	itArg++;
}

/**
 * @brief Sets the real name of the client after validating the input.
 *
 * This function extracts the real name from the provided arguments and performs
 * validation checks to ensure it is correctly formatted and valid. If the real name
 * is invalid, it throws an exception and sets the client's username to an empty string.
 *
 * @param itArg Iterator pointing to the current argument in the list of arguments.
 * @param args A vector of strings containing the command arguments.
 *
 * @throws std::invalid_argument if the real name is invalid or improperly formatted.
 */
void CommandHandler::_realNameSettings(std::vector<std::string>::iterator& itArg, const std::vector<std::string>& args)
{
	std::string realName = Utils::stockVector(++itArg, args);

	if ((realName[0] != ':') || (realName[0] == ':' && realName.size() == 1))
	{
		_client->setUsername("");
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), USER));
	}
	realName.erase(0, 1);

	if (IrcHelper::isValidName(realName, name_type::REALNAME) == false)
	{
		_client->setUsername("");
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), USER));
	}
	_client->setRealName(realName);
}

/**
 * @brief Handles the CAP (Capabilities) command for the client.
 *
 * This function processes the CAP command, which is used to negotiate
 * capabilities between the client and the server. It supports the "LS" and
 * "END" subcommands.
 *
 * - If the client is authenticated, it uses the client's nickname; otherwise,
 *   it uses "*".
 * - If the argument is not "LS" or "END", it throws an invalid_argument
 *   exception indicating that more parameters are needed.
 * - If the client is not marked as using Irssi, it sets the client to use Irssi.
 * - If the argument is "LS", it sends the capabilities message to the client
 *   and marks the client as identified.
 * - If the argument is "END", it marks the client as not identified.
 *
 * @throws std::invalid_argument if more parameters are needed or if the argument
 *         is not "LS" or "END".
 */
void CommandHandler::_handleCapabilities()
{
	std::string nickname = _client->isAuthenticated() ? _client->getNickname() : "*";
	std::string arg = *_itInput;

	if (arg != "LS" && arg != "END")
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(nickname, CAP));

	if (!_client->isIrssi())
		_client->setIsIrssi(true);

	if (arg == "LS") {
		_client->sendMessage(MessageBuilder::ircCapabilities(arg), NULL);
		_client->setIdentified(true);
	}
	if (arg == "END")
		_client->setIdentified(false);
}