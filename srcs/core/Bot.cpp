#include "../../incs/classes/Bot.hpp"

// === NAMESPACES ===
using namespace irc_replies;
using namespace server_messages;

// =========================================================================================

// --- PUBLIC
Bot::Bot(int botFd, const std::string& nick, const std::string& user, const std::string& real, Server& server)
	:  Client(botFd), _server(server), _clients(_server.getClients()), _channels(_server.getChannels())
{
	_authenticated = true;
	_clientSocketFd = botFd;
	_nickname = nick;
	_username = user;
	_realName = real;
	_hostname = nick;
	_clientIp = _hostname;
}
Bot::~Bot() {}


// === LISTEN ACTIVITY ===

/**
 * @brief Listens for activity on the server socket and processes input.
 *
 * This function runs an infinite loop that continuously checks for activity
 * on the server socket. It uses the `select` system call to monitor the socket
 * for readability with a timeout of 500 milliseconds. If a signal is received
 * from the server, the loop breaks and the function exits. If an error occurs
 * during the `select` call (other than an interrupt), a runtime exception is thrown.
 * When the server socket is ready for reading, the function calls `_readInput`
 * to process the incoming data.
 *
 * @throws std::runtime_error if an error occurs during the `select` call.
 */
void Bot::listenActivity()
{
	while (true)
	{
		if (_server.isSignalReceived())
			break;

		int serverSocketFd = _server.getServerSocketFd();
		fd_set readServerFd;
		FD_ZERO(&readServerFd);
		FD_SET(serverSocketFd, &readServerFd);
		struct timeval timeout = {0, 500000};

		// Attend un message du serveur
		int activity = select(serverSocketFd + 1, &readServerFd, NULL, NULL, &timeout);
		if (activity < 0 && errno != EINTR)
			throw std::runtime_error(ERR_SELECT_SOCKET);

		if (FD_ISSET(serverSocketFd, &readServerFd))
			_readInput();
	}
}


// --- PRIVATE

// === COMMAND HANDLER ===

/**
 * @brief Reads input from the server socket, processes the message, and handles commands.
 *
 * This function reads data from the server socket into a buffer, processes the message by
 * removing any trailing newline and carriage return characters, and then parses and handles
 * the command if the message is valid.
 *
 * @details
 * - Reads data from the server socket using `recv`.
 * - Removes trailing newline and carriage return characters from the message.
 * - Outputs the received message for debugging purposes.
 * - Parses the input message and handles the command if parsing is successful.
 *
 * @note If no data is read from the socket or an error occurs, the function returns immediately.
 */
void Bot::_readInput()
{
	char buffer[server::BUFFER_SIZE];
	std::memset(buffer, 0, sizeof(buffer));

	int bytesRead = recv(_server.getServerSocketFd(), buffer, sizeof(buffer) - 1, 0);
	if (bytesRead <= 0)
		return;

	buffer[bytesRead] = '\0';

	// Suppression \r\n
	std::string message(buffer);
	size_t pos = message.find('\n');
	std::string message = message.substr(0, pos);
	if (!message.empty() && message[message.size() - 1] == '\r')
		message.erase(message.size() - 1);

	// // Debug : affiche le message reçu
	std::cout << "---> bot received: " << message << std::endl;

	if (_parseInput(message) == false)
		return;
	std::string response = _handleCommand(message);
	sendMessage(response, this);
}

/**
 * @brief Parses the input string and extracts relevant information for the bot.
 *
 * This function processes the input string to extract the client's nickname, 
 * the command, and the message. It validates the input format and checks if 
 * the command is a PRIVMSG. If the input is invalid or the command is unknown, 
 * it sends an appropriate error message to the client.
 *
 * @param input The input string to be parsed.
 * @return true if the input is successfully parsed and valid, false otherwise.
 */
bool Bot::_parseInput(std::string& input)
{
	std::string errorMessage;

	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() != 4)
		_client->sendMessage(MessageHandler::ircNeedMoreParams(_nickname, input), this);

	// On recupère le nickname de l'envoyeur
	std::vector<std::string>::iterator itArg = args.begin();
	std::string clientNick = *itArg;
	_clientFd = _server.getClientByNickname(clientNick, this);
	_client = _clients[_clientFd];

	// On recupère la commande (doit etre PRIVMSG)
	++itArg;
	// if (itArg == args.end())
	// 	_client->sendMessage(MessageHandler::ircUnknownCommand(_nickname, input), this);
	std::string command = *itArg;
	if (command != commands::PRIVMSG)
		_client->sendMessage(MessageHandler::ircUnknownCommand(_nickname, input), this);

	// On skip le nick du bot
	++itArg;

	// On recupère l'input du client
	++itArg;
	std::string message = Utils::stockVector(itArg, args);
	std::vector<std::string> args = Utils::getTokens(message, splitter::SENTENCE);
	if (args.size() < 1)
		_client->sendMessage(MessageHandler::ircUnknownCommand(_nickname, input), this);
	
	// On recupère la commande bot
	std::vector<std::string>::iterator itArg = args.begin();
	_command = *itArg;

	// On recupère les arguments de la commande bot s'il y en a
	++itArg;
	if (itArg != args.end())
		_ageArg = *itArg;
}

/**
 * @brief Handles the input command and returns the appropriate response.
 *
 * This function processes the input command and returns a response based on the command type.
 * It supports the following commands:
 * - bot::JOKE_CMD: Returns a joke.
 * - bot::AGE_CMD: Returns the bot's age.
 * - bot::TIME_CMD: Returns the current time.
 * - commands::PING: Returns the current time.
 * If the command is unknown, it returns an unknown command message.
 *
 * @param input The input command to be processed.
 * @return A string containing the response to the input command.
 */
std::string Bot::_handleCommand(std::string& input)
{
	if (_command == bot::JOKE_CMD)
		return _getJoke();
	if (_command == bot::AGE_CMD)
		return _getAge();
	if (_command == bot::TIME_CMD)
		return IrcHelper::getCurrentTime();
	if (_command == commands::PING)
		return IrcHelper::getCurrentTime();
	return MessageHandler::ircUnknownCommand(_client->getNickname(), input);
}


// === JOKE COMMAND ===

/**
 * @brief Retrieves a random joke.
 *
 * This function fetches quotes from a predefined path and returns a random quote.
 *
 * @return A random joke as a std::string.
 */
std::string Bot::_getJoke()
{
	_quotes = _getQuotes(bot::QUOTES_PATH);
	return _getRandomQuote();
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

/**
 * @brief Generates a random quote from the list of quotes.
 * 
 * This function seeds the random number generator with the current time,
 * then selects and returns a random quote from the _quotes vector.
 * 
 * @return A randomly selected quote from the _quotes vector.
 */
std::string Bot::_getRandomQuote()
{
	std::srand(static_cast<unsigned int>(std::time(NULL)));
	return _quotes[std::rand() % _quotes.size()];
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
	if (_parseAge())
		return MessageHandler::ircMsgToClient(_nickname, _client->getNickname(), INVALID_DATE_FORMAT);

	res = _ageCalculator();
	return res;
}

/**
 * @brief Parses and validates the age argument in the format "YYYY-MM-DD".
 * 
 * This function checks if the age argument (_ageArg) is a valid date in the format "YYYY-MM-DD".
 * It performs the following validations:
 * - The age argument is not empty.
 * - The age argument contains exactly three parts separated by hyphens.
 * - Each part of the age argument contains only digits.
 * - The year is between 1900 and 2024.
 * - If the year is 2024, the month must be January or earlier.
 * - The month is between 1 and 12.
 * - The day is between 1 and 31.
 * - For months with 30 days (April, June, September, November), the day must be 30 or less.
 * - For February, the day must be 29 or less in leap years, and 28 or less in non-leap years.
 * 
 * @return true if the age argument is valid, false otherwise.
 */
bool Bot::_parseAge()
{
	if (_ageArg.empty())
		return false;

	std::string year, month, day, age;
	age = _ageArg;
	std::stringstream ss(_ageArg);

	int flag = 0;
	while (std::getline(ss, _ageArg, '-'))
	{
		flag++;
		for (size_t i = 0; i < _ageArg.size(); ++i)
			if (!isdigit(_ageArg[i]) && _ageArg[i] != '-') 
				return false;
	}
	if (flag != 3) 
		return false;

	int found = age.find("-");
	year = age.substr(0, found);
	age = age.substr(found + 1);
	found = age.find("-");
	month = age.substr(0, found);
	day = age.substr(found + 1);

	if (std::atoi(year.c_str()) > 2024 || std::atoi(year.c_str()) < 1900)  
		return false;
	if (std::atoi(year.c_str()) == 2024 && std::atoi(month.c_str()) > 1) 
		return false;
	if (std::atoi(month.c_str()) > 12 || std::atoi(month.c_str()) < 1 || std::atoi(day.c_str()) > 31 || std::atoi(day.c_str()) < 1 ) 
		return false;
	if (std::atoi(month.c_str()) == 4 || std::atoi(month.c_str()) == 6 || std::atoi(month.c_str()) == 9 || std::atoi(month.c_str()) == 11)
		if (std::atoi(day.c_str()) > 30) 
			return false;
	if ((std::atoi(year.c_str()) % 4 == 0 && std::atoi(year.c_str()) % 100 != 0) || std::atoi(year.c_str()) % 400 == 0)
		if (std::atoi(month.c_str()) == 2 && std::atoi(day.c_str()) > 29) 
			return false;
	else if (std::atoi(month.c_str()) == 2 && std::atoi(day.c_str()) > 28) 
		return false;
	return true;
}

/**
 * @brief Calculates the age based on the provided birth date.
 *
 * This function calculates the age in years, months, and days from the birth date
 * provided in the _ageArg member variable. The birth date is expected to be in the
 * format "YYYY-MM-DD".
 *
 * @return A string representing the age in a formatted manner using MessageHandler::botGetAge.
 */
std::string Bot::_ageCalculator()
{
	int year, month, day;
	year = std::atoi(_ageArg.substr(0, 4).c_str());
	month = std::atoi(_ageArg.substr(5, 2).c_str());
	day = std::atoi(_ageArg.substr(8, 2).c_str());

	std::tm birthDate;
	memset(&birthDate, 0, sizeof(birthDate));
	birthDate.tm_year = year - 1900;
	birthDate.tm_mon = month - 1;
	birthDate.tm_mday = day;

	std::time_t birthTime = mktime(&birthDate);
	std::time_t currentTime;
	std::time(&currentTime);

	double seconds = difftime(currentTime, birthTime);
	int years = static_cast<int>(seconds / (365.25 * 24 * 60 * 60));
	int months = static_cast<int>((seconds - years * 365.25 * 24 * 60 * 60) / (30.44 * 24 * 60 * 60));
	int days = static_cast<int>((seconds - years * 365.25 * 24 * 60 * 60 - months * 30.44 * 24 * 60 * 60) / (24 * 60 * 60));
	
	return MessageHandler::botGetAge(years, months, days);
}