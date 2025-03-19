#include "../../incs/classes/Bot.hpp"

// === NAMESPACES ===
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

// === INIT AND LISTEN ===

void Bot::_initBot()
{
	// std::string welcome;
	// ssize_t recivedBytes;
	// std::string nickname, command;

	// _Loggedin = false;
	// char buffer[1024];
	// std::vector<std::string> buff;
	// std::string botmask = nick + "!" + '~' + user + "@" + adr;
	// welcome = MessageHandler::ircWelcomeMessage(user, botmask);
	// while(true)
	// {
	// 	bzero(buffer, sizeof(buffer));
	// 	recivedBytes = recv(sock, buffer, (sizeof(buffer) - 1), 0);
	// 	if(recivedBytes <= 0)
	// 	{
	// 		perror("Bot :");
	// 		return ;
	// 	}
	// 	if (buffer[0])
	// 		buff.push_back(std::string(buffer));
	// 	while(!buff.empty() && !_Loggedin)
	// 	{
	// 		if (buff.back().find(welcome, 0) == std::string::npos)
	// 			buff.pop_back();
	// 		else
	// 		{
	// 			std::cout << "\nBOT Is Connected!\n" << std::endl;
	// 			_Loggedin = true;
	// 		}
	// 	}
	// 	if (!_Loggedin)
	// 	{
	// 		std::cout << "Error while connecting." << std::endl;
	// 		return ;
	// 	}
	// 	else if (buff.back().find(MessageHandler::ircPing()) != std::string::npos)
	// 	{
	// 		std::stringstream ss;
	// 		ss << MessageHandler::ircPong();
	// 		sendMessage(ss.str(), sock);
	// 	}
	// 	else if(buff.back().find("PRIVMSG") != std::string::npos && _Loggedin)
	// 	{
	// 		if (commandHandler(buff, nickname, command, sock) == 1)
	// 			continue ;
	// 	}
	// 	else if (buff.back().find("NOTICE") != std::string::npos && _Loggedin && buff.back().find("PRIVMSG") == std::string::npos)
	// 	{
	// 		if (getInviteHandler(buff, nickname, command, sock) == 1)
	// 			continue;
	// 	}
	// }
}

void Bot::_listenActivity()
{
	while (1)
	{
		if (_server.isSignalReceived())
			break;
	
		fd_set readFds = _server.getReadFds();
		int maxFd = _server.getMaxFd();
		struct timeval timeout = {0, 500000};
	
		if (select(maxFd + 1, &readFds, NULL, NULL, &timeout) < 0 && errno != EINTR)
			throw std::runtime_error(ERR_SELECT_SOCKET);
	
		for (int fd = 0; fd <= maxFd; fd++)
		{
			if (_server.isSignalReceived())
				break;
			if (FD_ISSET(fd, &readFds))
			{
				if (fd != _server.getServerSocketFd())
				{
					std::map<int, Client*>::iterator it = _clients.find(fd);
					if (it != _clients.end())
					{
						_client = it->second;
						_clientFd = _client->getFd();
						_readInput();
					}
				}
	
			}
		}
	}
}

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
	_handleCommand(message);
}

void Bot::_parseInput(std::string& input)
{
	std::vector<std::string> args = Utils::getTokens(input, splitter::SENTENCE);
	if (args.size() < 1)
		throw std::invalid_argument(ERR_INVALID_CMD_FORMAT);
	std::vector<std::string>::iterator itArg = args.begin();
	_command = *itArg;
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