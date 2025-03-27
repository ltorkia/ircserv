#include "../../incs/bot/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/utils/Utils.hpp"

// === NAMESPACES ===
#include "../../incs/config/bot_config.hpp"
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/commands.hpp"

using namespace bot_config;

// =========================================================================================

// === COMMAND PARSER ===

// ========================================= PRIVATE =======================================

// === SERVER COMMAND PARSER ===

/**
 * @brief Parses a PRIVMSG command from the input string.
 *
 * This function processes an input string to determine if it contains a valid
 * PRIVMSG command. It extracts the sender's nickname, verifies the command,
 * extracts the target and message, and then attempts to parse the bot command
 * from the message. If the message does not contain a valid bot command, it
 * announces the bot's features to the client.
 *
 * @param input The input string containing the PRIVMSG command.
 * @return true if the PRIVMSG command is successfully parsed and contains a valid bot command.
 * @return false if the input is invalid, the command is incorrect, the target or message cannot be extracted,
 *         or the message does not contain a valid bot command.
 */
bool Bot::_parsePrivmsg(std::string& input)
{
	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() < 4)
		return false;

	if (!_extractSenderNick(args.front())
		|| !_isRightCommand(args, commands::PRIVMSG)
		|| !_extractTarget(args))
		return false;

	std::string message = _extractMessage(args);
	if (message.empty())
		return false;

	if (!_parseBotCommand(message))
	{
		// Envoi d'un message au client pour lister les fonctionnalités du bot.
		// Il n'est envoyé qu'une fois en message privé.
		if (!_clientNickname.empty() && _target == _clientNickname)
			_announceBotFeatures();
		return false;
	}
	return true;
}

/**
 * @brief Extracts the sender's nickname from the given string.
 *
 * This function processes the input nickname string by removing the leading
 * colon character (':') if present. It then assigns the processed nickname
 * to the _clientNickname member variable and prints it to the standard output.
 *
 * @param nickname A reference to the string containing the sender's nickname.
 *                 This string may start with a colon character (':').
 * @return true if the extracted nickname is not empty and is different from
 *         the server's name; false otherwise.
 */
bool Bot::_extractSenderNick(std::string& nickname)
{
	if (!nickname.empty() && nickname[0] == ':')
		nickname.erase(0, 1);
	_clientNickname = nickname;

	if (_clientNickname.empty() || _clientNickname == server::NAME)
		return false;

	_target = _clientNickname;
	return true;
}

/**
 * @brief Checks if the given command matches the expected command.
 * 
 * This function compares the second element of the args vector with the provided command string.
 * 
 * @param args A vector of strings containing the command arguments.
 * @param command The command string to be validated against.
 * @return true if the second element of args matches the command string, false otherwise.
 */
bool Bot::_isRightCommand(const std::vector<std::string>& args, const std::string& command)
{
	return args[1] == command;
}

/**
 * @brief Extracts the target from the given arguments.
 * 
 * This function processes the provided vector of strings to extract the target
 * information required by the bot. The target is typically a specific user or
 * channel that the bot needs to interact with.
 * 
 * @param args A vector of strings containing the arguments from which the target
 *             needs to be extracted.
 * @return true if the target was successfully extracted, false otherwise.
 */
bool Bot::_extractTarget(const std::vector<std::string>& args)
{
	_channelName = (args[2] != _botNick) ? args[2] : "";
	_target = !_channelName.empty() ? _channelName : _clientNickname;

	if (_target.empty())
		return false;

	return true;
}

/**
 * @brief Extracts a message from a vector of arguments.
 *
 * This function takes a vector of strings as input and extracts a message
 * starting from the fourth element. If the extracted message starts with a
 * colon (':'), the colon is removed. The resulting message is then printed
 * to the standard output and returned.
 *
 * @param args A reference to a vector of strings containing the arguments.
 * @return A string containing the extracted message.
 */
std::string Bot::_extractMessage(std::vector<std::string>& args)
{
	std::vector<std::string>::iterator itArg = args.begin() + 3;
	std::string message = Utils::stockVector(itArg, args);
	if (!message.empty() && message[0] == ':')
		message.erase(0, 1);
	return message;
}

// =========================================================================================

// === BOT COMMAND PARSER ===

/**
 * @brief Parses a bot command from the given message.
 *
 * This function extracts the command and its potential arguments from the message.
 * It verifies if the command is valid and, if the command is "AGE", it stores the argument.
 *
 * @param message The message containing the bot command to be parsed.
 * @return true if the command is successfully parsed and valid, false otherwise.
 */
bool Bot::_parseBotCommand(std::string& message)
{
	// On extrait la commande et ses potentiels arguments du message
	message = message.substr(_commandPos);
	std::vector<std::string> argsInput = Utils::getTokens(message, splitter::SENTENCE);
	if (argsInput.empty())
		return false;

	// On vérifie que la commande stockée est valide
	_command = argsInput.front();
	if (!_isValidBotCommand(_command))
		return false;

	// Si la commande est AGE, un argument est attendu, on stocke le reste de la string
	if (_command == AGE_CMD && argsInput.size() > 1)
		_ageArg = argsInput[1];

	return true;
}

/**
 * @brief Checks if the given command is a valid bot command.
 *
 * This function compares the input command against predefined bot commands
 * such as FUNFACT_CMD, AGE_CMD, and TIME_CMD to determine if it is valid.
 *
 * @param command The command string to be validated.
 * @return true if the command is valid, false otherwise.
 */
bool Bot::_isValidBotCommand(const std::string& command)
{
	return command == FUNFACT_CMD || command == AGE_CMD || command == TIME_CMD;
}

// === AGE COMMAND ===

/**
 * @brief Parses and validates the birthdate string stored in _ageArg.
 *
 * This function checks if the birthdate string (_ageArg) is in the format "YYYY-MM-DD".
 * If the string is empty, too short, or does not follow the expected format, the function returns false.
 * It then extracts the first 10 characters of the string and retrieves the current date.
 * The function proceeds to extract the year, month, and day from the birthdate string and validates these values.
 *
 * @return true if the birthdate string is valid and correctly parsed, false otherwise.
 */
bool Bot::_parseBirthdate()
{
	// Vérifier que la chaîne à traiter respecte le format "YYYY-MM-DD"
	// Si la string continue après, vérifier qu'il y a bien un espace après la date
	if (_ageArg.empty() || _ageArg.size() < 10 || _ageArg[4] != '-'
	|| _ageArg[7] != '-' || (_ageArg.size() > 10 && !isspace(_ageArg[10])))
		return false;

	// Ne conserver que les 10 premiers caractères
	_ageArg = _ageArg.substr(0, 10);

	// Récupérer la date actuelle
	std::tm now;
	Utils::getCurrentTime(now);

	_currentYear = now.tm_year + 1900;
	_currentMonth = now.tm_mon + 1;
	_currentDay = now.tm_mday;

	// Extraire année, mois, jour, et vérifications des valeurs
	if (!_extractDate(_ageArg) || !_isValidDate())
		return false;

	return true;
}

/**
 * @brief Extracts the date from a given string and stores it in the Bot object.
 *
 * This function takes a date string in the format "YYYY-MM-DD" and extracts the year, month,
 * and day from it. It then converts these values to integers and stores them in the Bot object.
 *
 * @param dateStr The date string in the format "YYYY-MM-DD".
 * @return true if the date was successfully extracted and stored, false otherwise.
 */
bool Bot::_extractDate(const std::string& dateStr)
{
	if (dateStr.size() != 10)
		return false;

	// Extraire année, mois, jour
	std::string yearStr = dateStr.substr(0, 4);
	std::string monthStr = dateStr.substr(5, 2);
	std::string dayStr = dateStr.substr(8, 2);

	if (!Utils::isNumber(yearStr) || !Utils::isNumber(monthStr) || !Utils::isNumber(dayStr))
		return false;

	// Convertir en entiers
	_year = std::atoi(yearStr.c_str());
	_month = std::atoi(monthStr.c_str());
	_day = std::atoi(dayStr.c_str());
	return true;
}

/**
 * @brief Checks if the date stored in the Bot object is valid.
 *
 * This function performs several checks to ensure the date is valid:
 * - The year must be between 1900 and the current year.
 * - The month must be between 1 and 12.
 * - The day must be between 1 and 31.
 * - The date must not be in the future.
 * - February is checked for leap years.
 * - Months with 30 days are validated.
 *
 * @return true if the date is valid, false otherwise.
 */
bool Bot::_isValidDate()
{
	// Vérification de l'année, du moi et du jour
	if (_year < 1900 || _year > _currentYear
		|| _month < 1 || _month > 12
		|| _day < 1 || _day > 31)
		return false;

	// Vérification que la date n'est pas dans le futur
	if ((_year == _currentYear && _month > _currentMonth)
		|| (_year == _currentYear && _month == _currentMonth && _day > _currentDay))
		return false;

	bool isLeapYear = (_year % 4 == 0 && _year % 100 != 0) || (_year % 400 == 0);

	// Vérifier février (année bissextile)
	if (_month == 2)
		return _day <= (isLeapYear ? 29 : 28);

	// Vérifier les mois à 30 jours
	if (_month == 4 || _month == 6 || _month == 9 || _month == 11)
		return _day <= 30;

	return true;
}