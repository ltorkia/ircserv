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


// --- PRIVATE

// === LISTEN ACTIVITY ===

void Bot::_listenActivity()
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


// === COMMAND HANDLER ===

void Bot::_readInput()
{
	char buffer[server::BUFFER_SIZE];
	std::memset(buffer, 0, sizeof(buffer));

	int bytesRead = recv(_server.getServerSocketFd(), buffer, sizeof(buffer) - 1, 0);

	if (bytesRead <= 0)
		throw std::runtime_error(ERR_READ_SERVER);

	buffer[bytesRead] = '\0';

	std::string message(buffer);

	std::cout << "Bot received: " << message << std::endl;

	// std::string bufferMessage(buffer);
	// size_t pos = bufferMessage.find('\n');
	// std::string message = bufferMessage.substr(0, pos);

	// if (!message.empty() && message[message.size() - 1] == '\r')
	// 	message.erase(message.size() - 1);

	// // Debug : affiche le message reçu
	// // std::cout << "---> " << message << std::endl;

	_parseInput(message);
	std::string response = _handleCommand(message);
	sendMessage(response, this);
}

void Bot::_parseInput(std::string& input)
{
	std::string errorMessage;

	std::vector<std::string> args = Utils::getTokens(input, splitter::WORD);
	if (args.size() != 4)
		errorMessage = ERR_NEEDMOREPARAMS_MSG;

	// On skip le nickname bot et on recupère la commande (doit etre PRIVMSG)
	std::vector<std::string>::iterator itArg = args.begin();
	++itArg;
	std::string command = *itArg;

	// On recupère le nickname du client
	++itArg;
	std::string clientNick = *itArg;
	_clientFd = _server.getClientByNickname(clientNick, this);
	if (IrcHelper::clientExists(_clientFd) == false)
		return;
	_client = _clients[_clientFd];

	// Verification de la commande apres avoir identifié le client
	if (command != commands::PRIVMSG)
		_client->sendMessage(MessageHandler::ircUnknownCommand(clientNick, input), this);

	// On recupère l'input du client
	++itArg;
	std::string message = Utils::stockVector(itArg, args);
	std::vector<std::string> args = Utils::getTokens(message, splitter::SENTENCE);
	if (args.size() < 1)
		_client->sendMessage(MessageHandler::ircUnknownCommand(clientNick, input), this);
	
	// On recupère la commande bot
	std::vector<std::string>::iterator itArg = args.begin();
	_command = *itArg;

	// On recupère les arguments de la commande bot s'il y en a
	++itArg;
	if (itArg != args.end())
		_ageArg = *itArg;
}

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

std::string Bot::_getJoke()
{
	_quotes = _getQuotes(bot::QUOTES_PATH);
	return _getRandomQuote();
}

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

std::string Bot::_getRandomQuote()
{
	std::srand(static_cast<unsigned int>(std::time(NULL)));
	return _quotes[std::rand() % _quotes.size()];
}


// === AGE COMMAND ===

std::string Bot::_getAge()
{
	std::string res;
	if (_parseAge())
		throw std::invalid_argument(INVALID_DATE_FORMAT);

	res = _ageCalculator();
	return res;
}

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