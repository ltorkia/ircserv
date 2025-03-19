#include "../../incs/classes/Bot.hpp"

Bot::Bot(const std::string& nick, const std::string& user, const std::string& realName)
	: _nickname(nick), _username(user), _realName(realName), _hostname(nick), _ipAdress(server::LOCALHOST) {}
Bot::Bot() {}
Bot::Bot(const Bot& src) {(void) src;}
Bot & Bot::operator=(Bot& src) {(void) src; return *this;}
Bot::~Bot() {}

std::string Bot::getBotMask() const
{
	return _nickname + "!~" + _username + "@" + _ipAdress;
}

std::string Bot::handleCommand(const std::string& command)
{
	if (command == "!joke")
		return getJoke();
	if (command == "!time")
		return getJoke();
	if (command == "!age")
		return IrcHelper::getCurrentTime();
	return "Unknown command.";
	// throw std::invalid_argument(MessageHandler::ircUnknownCommand(nickname, stringSent));
}

std::string Bot::getJoke()
{
	return "Why do Java developers wear glasses? Because they don't see sharp!";
}

std::string Bot::getAge(const std::string &input)
{
	return "I am 1 year old!";
}

	/**
 * @brief Splits the input buffer string into two parts: the first word and the rest of the string.
 *
 * This function takes an input buffer string and splits it into two parts. The first word
 * (token) is returned, and the rest of the string is stored in the provided date reference.
 *
 * @param buff The input buffer string to be split.
 * @param date A reference to a string where the rest of the buffer will be stored.
 * @return The first word (token) from the input buffer string.
 */
std::string Bot::splitBuff(std::string buff, std::string &date)
{
	std::istringstream stm(buff);
	std::string token;
	stm >> token;
	stm >> date;
	return token;
}

/**
 * @brief Parses and validates a given age string in the format "YYYY-MM-DD".
 *
 * This function checks if the input string is a valid date in the format "YYYY-MM-DD".
 * It ensures that the year is between 1900 and 2024, the month is between 1 and 12,
 * and the day is valid for the given month, including leap year considerations.
 *
 * @param age The age string to be parsed and validated.
 * @return Returns 1 if the age string is valid, otherwise returns 0.
 */
int Bot::parseAge(std::string age)
{
	std::string year, month, day, age1;
	age1 = age;
	std::stringstream ss(age);

	int flag = 0;
	while (std::getline(ss, age, '-'))
	{
		flag++;
		for (size_t i = 0; i < age.size(); ++i)
		{
			if (!isdigit(age[i]) && age[i] != '-') 
				return 0;
		}
	}
	if (flag != 3) 
		return 0;

	int found = age1.find("-");
	year = age1.substr(0, found);
	age1 = age1.substr(found+1);
	found = age1.find("-");
	month = age1.substr(0, found);
	day = age1.substr(found+1);

	if (std::atoi(year.c_str()) > 2024 || std::atoi(year.c_str()) < 1900)  
		return 0;
	if (std::atoi(year.c_str()) == 2024 && std::atoi(month.c_str()) > 1) 
		return 0;
	if (std::atoi(month.c_str()) > 12 || std::atoi(month.c_str()) < 1 || std::atoi(day.c_str()) > 31 || std::atoi(day.c_str()) < 1 ) 
		return 0;
	if (std::atoi(month.c_str()) == 4 || std::atoi(month.c_str()) == 6 || std::atoi(month.c_str()) == 9 || std::atoi(month.c_str()) == 11)
	{
		if (std::atoi(day.c_str()) > 30) 
			return 0;
	}
	if ((std::atoi(year.c_str()) % 4 == 0 && std::atoi(year.c_str()) % 100 != 0) || std::atoi(year.c_str()) % 400 == 0)
	{
		if (std::atoi(month.c_str()) == 2 && std::atoi(day.c_str()) > 29) 
			return 0;
	}
	else
	{
			if (std::atoi(month.c_str()) == 2 && std::atoi(day.c_str()) > 28) 
				return 0;
	}
	return 1;
}

/**
 * @brief Calculates the age of a user based on their birth date and sends a message with the result.
 * 
 * @param age The birth date of the user in the format "year-month-day".
 * @param Nickname The nickname of the user to whom the message will be sent.
 * @param ircsock The IRC socket used to send the message.
 * 
 * This function parses the provided birth date, calculates the age in years, months, and days,
 * and sends a private message to the user with the calculated age. If the date format is invalid,
 * it sends an error message to the user.
 */
void Bot::ageCalculator(std::string age, std::string Nickname,int ircsock)
{
	if (!parseAge(age))
	{
		// Bot::send_privmsg("Invalid date format(!age <year-month-day>).", Nickname, ircsock);
		return;
	}

	int year, month, day;
	year = std::atoi(age.substr(0, 4).c_str());
	month = std::atoi(age.substr(5, 2).c_str());
	day = std::atoi(age.substr(8, 2).c_str());

	std::tm birth_date;
	memset(&birth_date, 0, sizeof(birth_date));
	birth_date.tm_year = year - 1900;
	birth_date.tm_mon = month - 1;
	birth_date.tm_mday = day;

	std::time_t birth_time = mktime(&birth_date);
	std::time_t current_time;
	std::time(&current_time);

	double seconds = difftime(current_time, birth_time);
	int years = static_cast<int>(seconds / (365.25 * 24 * 60 * 60));
	int months = static_cast<int>((seconds - years * 365.25 * 24 * 60 * 60) / (30.44 * 24 * 60 * 60));
	int days = static_cast<int>((seconds - years * 365.25 * 24 * 60 * 60 - months * 30.44 * 24 * 60 * 60) / (24 * 60 * 60));
	
	std::stringstream ss;
	ss << "You are : " << years << " years, " << months << " months, " << days << " days old";
	// Bot::send_privmsg(ss.str(), Nickname, ircsock);
}

/**
 * @brief Reads quotes from a file and stores them in the _quotes vector.
 * 
 * This function opens the specified file, reads each line, and stores it in the _quotes vector.
 * If the file cannot be opened, an error message is printed to the standard error stream.
 * 
 * @param filename The name of the file to read quotes from.
 * @return int Returns 1 if the file was successfully read, otherwise returns 0.
 */
int Bot::getQuotes(std::string filename)
{
	std::string line;
	std::ifstream file(filename.c_str());

	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;
		return 0;
	}
	while (std::getline(file, line))
		_quotes.push_back(line);
	file.close();
	return 1;
}

/**
 * @brief Selects a random quote from the provided list of quotes.
 *
 * This function initializes the random number generator with the current time,
 * then selects and returns a random quote from the given vector of quotes.
 *
 * @param _quotes A vector containing the list of quotes to choose from.
 * @param size The number of quotes in the vector.
 * @return A randomly selected quote from the vector.
 */
std::string Bot::getQuotes(std::vector<std::string>& quotes, int size)
{
	std::srand(static_cast<unsigned int>(std::time(NULL)));
	return quotes[std::rand() % size];
}