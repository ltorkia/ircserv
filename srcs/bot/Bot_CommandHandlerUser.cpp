#include "../../incs/classes/bot/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/utils/Utils.hpp"
#include "../../incs/classes/utils/IrcHelper.hpp"
#include "../../incs/classes/utils/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/bot.hpp"
#include "../../incs/config/irc_replies.hpp"
#include "../../incs/config/server_messages.hpp"

using namespace irc_replies;
using namespace server_messages;

// =========================================================================================

// === COMMAND HANDLER USER ===

/**
 * @brief Handles the input command and returns the appropriate response.
 *
 * This function processes the input command and returns a response based on the command type.
 * It supports the following commands:
 * - bot::FUNFACT_CMD: Returns a cultural funfact.
 * - bot::AGE_CMD: Returns the client's age.
 * - bot::TIME_CMD: Returns the current time.
 * If the command is unknown, it returns an unknown command message.
 *
 * @return A string containing the response to the input command.
 */
std::string Bot::_handleBotCommand()
{
	if (_command == bot::FUNFACT_CMD)
		return _getRandomFunfact();
	if (_command == bot::AGE_CMD)
		return _getAge();
	if (_command == bot::TIME_CMD)
		return IrcHelper::getTimeString();
	return ERR_UNKNOWNCOMMAND_MSG;
}


// === FACT COMMAND ===

/**
 * @brief Retrieves a random funfact from a list of quotes.
 *
 * This function loads a list of quotes from a predefined file path,
 * seeds the random number generator with the current time, and returns
 * a randomly selected quote from the list.
 *
 * @return A randomly selected funfact as a std::string.
 */
std::string Bot::_getRandomFunfact()
{
	if (_quotes.empty())
		_quotes = _getQuotes(bot::QUOTES_PATH);
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
	IrcHelper::getCurrentTime(now);

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
	return MessageHandler::botGetAge(years, months, days);
}