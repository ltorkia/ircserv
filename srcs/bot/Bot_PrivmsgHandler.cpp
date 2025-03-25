#include "../../incs/classes/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/IrcHelper.hpp"
#include "../../incs/classes/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_replies.hpp"
#include "../../incs/config/server_messages.hpp"

using namespace irc_replies;
using namespace server_messages;

// =========================================================================================

// === PRIVMSG HANDLER ===

/**
 * @brief Handles the input command and returns the appropriate response.
 *
 * This function processes the input command and returns a response based on the command type.
 * It supports the following commands:
 * - bot::JOKE_CMD: Returns a joke.
 * - bot::AGE_CMD: Returns the bot's age.
 * - bot::TIME_CMD: Returns the current time.
 * If the command is unknown, it returns an unknown command message.
 *
 * @return A string containing the response to the input command.
 */
std::string Bot::_handlePrivmsgCommand()
{
	if (_command == bot::JOKE_CMD)
		return _getRandomJoke();
	if (_command == bot::AGE_CMD)
		return _getAge();
	if (_command == bot::TIME_CMD)
		return IrcHelper::getTimeString();
	return ERR_UNKNOWNCOMMAND_MSG;
}


// === JOKE COMMAND ===

/**
 * @brief Retrieves a random joke from a list of quotes.
 *
 * This function loads a list of quotes from a predefined file path,
 * seeds the random number generator with the current time, and returns
 * a randomly selected quote from the list.
 *
 * @return A randomly selected joke as a std::string.
 */
std::string Bot::_getRandomJoke()
{
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
 * @brief Parses and validates the age argument in the format "YYYY-MM-DD".
 *
 * This function checks if the age argument (_ageArg) is a valid date string
 * in the format "YYYY-MM-DD". It performs the following validations:
 * - The string must be exactly 10 characters long and follow the "YYYY-MM-DD" format.
 * - The year, month, and day components must be numeric.
 * - The year must be between 1900 and the current year.
 * - The month must be between 1 and 12.
 * - The day must be valid for the given month (considering leap years for February).
 * - The date must not be in the future.
 *
 * @return true if the age argument is valid, false otherwise.
 */
bool Bot::_parseBirthdate()
{
	if (_ageArg.empty())
		return false;

	// Vérifier que la chaîne à traiter respecte le format "YYYY-MM-DD"
	if (_ageArg.size() < 10 || _ageArg[4] != '-' || _ageArg[7] != '-'
		|| (_ageArg.size() > 10 && !isspace(_ageArg[10])))
		return false;

	// Ne conserver que les 10 premiers caractères
	_ageArg = _ageArg.substr(0, 10);

	// Récupérer la date actuelle
	std::tm now;
	IrcHelper::getCurrentTime(now);

	int currentYear = now.tm_year + 1900;
	int currentMonth = now.tm_mon + 1;

	// Extraire année, mois, jour
	std::istringstream stream(_ageArg);
	std::string yearStr, monthStr, dayStr;

	std::getline(stream, yearStr, '-');
	std::getline(stream, monthStr, '-');
	std::getline(stream, dayStr);

	// Vérifier que tous les caractères sont numériques
	for (size_t i = 0; i < yearStr.size(); ++i)
		if (!std::isdigit(yearStr[i])) return false;
	for (size_t i = 0; i < monthStr.size(); ++i)
		if (!std::isdigit(monthStr[i])) return false;
	for (size_t i = 0; i < dayStr.size(); ++i)
		if (!std::isdigit(dayStr[i])) return false;

	// Convertir en entiers
	int year = std::atoi(yearStr.c_str());
	int month = std::atoi(monthStr.c_str());
	int day = std::atoi(dayStr.c_str());

	// Vérification de l'année
	if (year < 1900 || year > currentYear)
		return false;

	// Vérification du mois
	if (month < 1 || month > 12)
		return false;

	// Vérification du jour
	if (day < 1 || day > 31)
		return false;

	// Vérifier les mois à 30 jours
	if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30)
		return false;

	// Vérifier février (année bissextile)
	bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	if (month == 2)
	{
		if (isLeapYear && day > 29)
			return false;
		else if (!isLeapYear && day > 28)
			return false;
	}

	// Vérification que la date n'est pas dans le futur
	if (year == currentYear && month > currentMonth)
		return false;

	return true;
}

/**
 * @brief Calculates the age based on the provided birth date.
 *
 * This function calculates the age in years, months, and days from the birth date
 * provided in the _ageArg member variable. The birth date should be in the format "YYYY-MM-DD".
 *
 * @return A string representing the age in a specific format, as returned by MessageHandler::botGetAge.
 */
std::string Bot::_ageCalculator()
{
	int year, month, day;
	year = std::atoi(_ageArg.substr(0, 4).c_str());
	month = std::atoi(_ageArg.substr(5, 2).c_str());
	day = std::atoi(_ageArg.substr(8, 2).c_str());

	// Obtenir la date actuelle
	std::tm now;
	IrcHelper::getCurrentTime(now);

	// Calcul de l'âge en années
	int years = now.tm_year + 1900 - year;
	int months = now.tm_mon + 1 - month;
	int days = now.tm_mday - day;

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
		int prevMonth = (month == 1) ? 12 : month - 1;
		int prevYear = (month == 1) ? year - 1 : year;

		// Vérifier année bissextile pour février
		int daysInPrevMonth = daysInMonth[prevMonth - 1];
		if (prevMonth == 2 && ((prevYear % 4 == 0 && prevYear % 100 != 0) || (prevYear % 400 == 0)))
			daysInPrevMonth = 29;

		days += daysInPrevMonth;
		months--;
	}
	return MessageHandler::botGetAge(years, months, days);
}