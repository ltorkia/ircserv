#include "../../incs/bot/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/utils/Utils.hpp"
#include "../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../incs/config/bot_config.hpp"
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/server_messages.hpp"

using namespace bot_config;
using namespace server_messages;

// =========================================================================================

// === SERVER COMMAND HANDLER ===

/**
 * @brief Manages the incoming command message for the bot.
 *
 * This function processes the incoming message and determines the appropriate
 * action to take based on the command contained within the message. It handles
 * various commands such as ping, invite, and join. If none of these commands
 * are detected, it parses the message as a private message command and sends
 * the appropriate response.
 *
 * @param message The incoming command message to be managed.
 */
void Bot::_manageServerCommand(std::string& message)
{
	if (_handlePing(message) || _handleInvite(message)
		|| _handleJoin(message) || !_parsePrivmsg(message))
		return;

	std::string response = _handleBotCommand();
	_sendMessage(MessageBuilder::botCmdPrivmsg(_target, response));
}

/**
 * @brief Handles the PING command from the IRC server.
 *
 * This function checks if the input message is a PING command. If it is,
 * it responds with a PONG message.
 *
 * @param input The input message to be checked.
 * @return true if the input message is a PING command and a PONG response
 *         was sent, false otherwise.
 */
bool Bot::_handlePing(const std::string& input)
{
	if (input != MessageBuilder::ircPing())
		return false;
	_sendMessage(MessageBuilder::ircPong());
	return true;
}

/**
 * @brief Handles an invite message and joins the specified channel.
 *
 * This function processes an invite message, verifies its format, extracts the
 * channel name, and sends a JOIN command to the server. It also announces the
 * bot's features to the clients in the channel.
 *
 * @param input The invite message received.
 * @return true if the invite message is successfully processed and the bot joins the channel.
 * @return false if the invite message is invalid or processing fails.
 */
bool Bot::_handleInvite(const std::string& input)
{
	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() < 4)
		return false;

	// Vérification du format du message
	if (args[0] != ":" + server::NAME
		|| !_isRightCommand(args, NOTICE_CMD)
		|| args[2] != server::NAME)
		return false;

	// Extraction du message d'invitation
	std::string message = _extractMessage(args);
	if (message.empty())
		return false;

	// On cherche la position du nom du channel
	size_t channelPos = message.find("#");
	if (channelPos == std::string::npos)
		return false;

	// Extraction du nom du channel, 
	// suppression des caractères non imprimables,
	// et envoi de la commande JOIN
	_channelName = message.substr(channelPos, message.length() - channelPos);
	_channelName.erase(std::remove_if(_channelName.begin(), _channelName.end(), Utils::isNonPrintableChar), _channelName.end());
	_target = _channelName;
	_sendMessage(MessageBuilder::botCmdJoinChannel(_target));

	sleep(1);

	// Envoi d'un message aux clients du channel pour lister les fonctionnalités du bot
	_announceBotFeatures();
	
	return true;
}

/**
 * @brief Handles the JOIN command for the bot.
 *
 * This function processes the input string to handle the JOIN command.
 * It verifies the format of the message, checks if the command is valid,
 * and sends a message to the client who just joined the channel to list
 * the bot's features.
 *
 * @param input The input string containing the command and its arguments.
 * @return true if the JOIN command was successfully handled, false otherwise.
 */
bool Bot::_handleJoin(const std::string& input)
{
	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() < 3)
		return false;

	// Vérification du format du message
	if (args[1] != commands::JOIN)
		return false;

	if (!_extractSenderNick(args.front())
		|| !_isRightCommand(args, commands::JOIN)
		|| !_extractTarget(args))
		return false;

	// Envoi d'un message au client qui vient de join le channel
	// pour lister les fonctionnalités du bot
	_announceBotFeatures();

	return true;
}

// =========================================================================================

// === BOT COMMAND HANDLER ===

/**
 * @brief Handles the input command and returns the appropriate response.
 *
 * This function processes the input command and returns a response based on the command type.
 * It supports the following commands:
 * - FUNFACT_CMD: Returns a cultural fun fact.
 * - AGE_CMD: Returns the client's age.
 * - TIME_CMD: Returns the current time.
 * If the command is unknown, it returns an unknown command message.
 *
 * @return A string containing the response to the input command.
 */
std::string Bot::_handleBotCommand()
{
	if (_command == FUNFACT_CMD)
		return _getRandomFunfact();
	if (_command == AGE_CMD)
		return _getAge();
	if (_command == TIME_CMD)
		return _getTimeString();
	return ERR_UNKNOWN_BOT_CMD;
}


// === FUN FACT COMMAND ===

/**
 * @brief Retrieves a random fun fact from a list of quotes.
 *
 * This function loads a list of quotes from a predefined file path,
 * seeds the random number generator with the current time, and returns
 * a randomly selected quote from the list.
 *
 * @return A randomly selected fun fact as a std::string.
 */
std::string Bot::_getRandomFunfact()
{
	if (_quotes.empty())
		_quotes = _getQuotes(QUOTES_PATH);
	std::srand(static_cast<unsigned int>(std::time(NULL)));
	return _quotes[std::rand() % _quotes.size()];
}

/**
 * @brief Reads quotes from a file and returns them as a vector of strings.
 *
 * This function opens the specified file, reads each line, and stores it in a vector.
 * If the file cannot be opened, it throws an invalid_argument exception.
 *
 * @param filename The path to the file containing the quotes.
 * @return A vector of strings, each representing a quote from the file.
 * @throws std::invalid_argument If the file cannot be opened.
 */
std::vector<std::string> Bot::_getQuotes(std::string filename)
{
	std::vector<std::string> res;
	std::string line;
	std::ifstream file(filename.c_str());

	if (!file.is_open())
		throw std::invalid_argument(ERR_OPEN_FILE);

	while (std::getline(file, line))
		res.push_back(line);
	file.close();

	return res;
}


// === AGE COMMAND ===

/**
 * @brief Retrieves the age of the bot.
 *
 * This function calculates the age of the bot and returns it as a string.
 * If the age cannot be parsed correctly, it sends an error message to the client
 * indicating an invalid date format.
 *
 * @return std::string The calculated age of the bot, or an error message if the date format is invalid.
 */
std::string Bot::_getAge()
{
	std::string res;
	if (_parseBirthdate() == false)
		return INVALID_DATE_FORMAT;

	res = _ageCalculator();
	return res;
}

/**
 * @brief Calculates the age in years, months, and days based on the current date and the birth date.
 *
 * This function computes the age by subtracting the birth date from the current date.
 * It adjusts the calculation if the birthday has not occurred yet in the current year.
 * It also handles the case where the number of days is negative by borrowing days from the previous month.
 *
 * @return A string representing the age in the format provided by MessageHandler::botGetAge.
 */
std::string Bot::_ageCalculator()
{
	// Calcul de l'âge en années, mois, jours
	int years = _currentYear - _year;
	int months = _currentMonth - _month;
	int days = _currentDay - _day;

	// Ajustement si l'anniversaire n'est pas encore passé cette année
	if (months < 0 || (months == 0 && days < 0))
	{
		years--;
		months += 12;
	}
	if (days < 0)
	{
		// Trouver le nombre de jours dans le mois précédent
		static const int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		int prevMonth = (_month == 1) ? 12 : _month - 1;
		int prevYear = (_month == 1) ? _year - 1 : _year;

		// Vérifier année bissextile pour février
		int daysInPrevMonth = daysInMonth[prevMonth - 1];
		if (prevMonth == 2 && ((prevYear % 4 == 0 && prevYear % 100 != 0) || (prevYear % 400 == 0)))
			daysInPrevMonth = 29;

		days += daysInPrevMonth;
		months--;
	}
	return MessageBuilder::botGetAge(years, months, days);
}


// === TIME COMMAND ===

/**
 * @brief Get the current time as a string.
 * 
 * This function retrieves the current system time and returns it as a 
 * human-readable string. The format of the returned string is determined 
 * by the ctime function.
 * 
 * @return std::string The current time as a string.
 */
std::string Bot::_getTimeString()
{
	time_t now = time(0);
	return ctime(&now);
}