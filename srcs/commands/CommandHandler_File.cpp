#include "../../incs/classes/CommandHandler.hpp"
#include "../../incs/classes/CommandHandler_File.hpp"

//---------------------------------------------------FILE METHODS---------------------------------------------------//

File::File() {}
File::File( std::string Name, std::string Path, std::string Sender, std::string Receiver ): Name(Name), Path(Path), sender(Sender), receiver(Receiver) {}
File::File( const File &x ) {*this = x;}
File::~File() {}
File & File::operator=( const File &src )
{
	if (this == &src)
		return (*this);
	this->Name = src.Name;
	this->Path = src.Path;
	this->sender = src.sender;
	this->receiver = src.receiver;
	return (*this);
};

//---------------------------------------------------REQUEST METHODS---------------------------------------------------//

Request::Request(std::vector<std::string> arg, std::string cmd): args(arg), command(cmd) {};
Request::Request( const Request& x ) { *this = x; };
Request::~Request() {};
Request & Request::operator= ( const Request& src )
{
	if (this == &src)
		return (*this);
	this->args = src.args;
	this->command = src.command;
	return (*this);
};

//---------------------------------------------------DISTRIBUTION METHOD---------------------------------------------------//

void CommandHandler::_handleFile()
{
	std::vector<std::string> entry = Utils::getTokens(*_itInput, splitter::SENTENCE);
	if (entry.front() == "SEND")
		_sendFile(entry);
	else if (entry.front() == "GET")
		_getFile(entry);
}

//---------------------------------------------------SEND FILE METHODS---------------------------------------------------//

/**
 * @brief Handles the SEND command to transfer a file to another client.
 *
 * This function processes the SEND command by parsing the input entry, validating the request,
 * and sending the file to the specified client. It performs the following steps:
 * 1. Tokenizes the last entry to extract the request parameters.
 * 2. Validates the number of arguments in the request.
 * 3. Iterates through the request arguments to send the file to the specified client.
 * 4. Checks if the target client exists and if the file can be opened.
 * 5. Sends the file to the target client and updates the server's file list.
 *
 * @param entry A vector of strings representing the command and its arguments.
 */
void CommandHandler::_sendFile(std::vector<std::string> entry)
{
	if (chdir(getenv("HOME")) != 0) 
	{
		std::cerr << "Erreur : Impossible de changer de répertoire !" << std::endl;
		return ;
	}

	std::vector<std::string> req = Utils::getTokens(entry.back(), splitter::WORD);
	if (req.empty())
	{
		_client->sendMessage(MessageHandler::ircNeedMoreParams(_client->getNickname(), "SEND"), NULL);
		return ;
	}
	Request	request(req, "SEND");
	if (request.args.size() < 2)
	{
		_client->sendMessage(MessageHandler::ircNeedMoreParams(_client->getNickname(), "SEND"), NULL);
		return ;
	}

	while (request.args.size() >= 2)
	{
		int clientFd = _server.getClientByNickname(request.args[0], _client);
		if (IrcHelper::clientExists(clientFd) == false)
		{
			_client->sendMessage(MessageHandler::ircNoSuchNick(_client->getNickname(), request.args[1]), NULL);
			return ;
		}
		std::fstream ifs(request.args[1].c_str(), std::fstream::in);
		if (ifs.fail())
		{
			_client->sendMessage(MessageHandler::errorMsgSendFile(request.args[1]), NULL);
			return ;
		}
		size_t pos = request.args[1].find_last_of('/');
		std::string filename = request.args[1].substr(pos + 1);
		File file(filename, request.args[1], _client->getNickname(), request.args[0]);
		_server.getFiles().insert(std::pair<std::string, File>(filename, file));
		_client->sendMessage("DCC SEND request sent to " + request.args[0] + ": " + filename + eol::IRC, NULL);
		_clients[clientFd]->sendMessage(MessageHandler::msgSendFile(filename, _client->getNickname(), _client->getClientIp(), _client->getClientPort()), _client);
		request.args.erase(request.args.begin()+1);
	}
}

//---------------------------------------------------GET FILE METHODS---------------------------------------------------//

/**
 * @brief Handles the GET command to retrieve a file from another client.
 *
 * This function processes the GET command to retrieve a file that has been offered by another client.
 * It validates the request, checks if the file exists, and then transfers the file from the sender to the receiver.
 *
 * @param entry A vector of strings containing the command and its arguments.
 *
 * The function performs the following steps:
 * 1. Tokenizes the last entry in the command to extract the request parameters.
 * 2. Validates that the request has the necessary parameters.
 * 3. Checks if the specified file exists and if the sender and receiver match.
 * 4. Transfers the file from the sender to the receiver.
 * 5. Sends appropriate messages to the clients involved in the file transfer.
 */
void	CommandHandler::_getFile(std::vector<std::string> entry)
{
	if (chdir(getenv("HOME")) != 0) {
		std::cerr << "Erreur : Impossible de changer de répertoire !" << std::endl;
		return ;
	}
	std::vector<std::string> req = Utils::getTokens(entry.back(), splitter::WORD);
	if (req.empty())
	{
		_client->sendMessage(MessageHandler::ircNeedMoreParams(_client->getNickname(), "GET"), NULL);
		return ;
	}
	Request	request(req, "GET");
	if (request.args.size() < 2)
	{
		_client->sendMessage(MessageHandler::ircNeedMoreParams(_client->getNickname(), "GET"), NULL);
		return ;
	}
	std::map<std::string, File> non = _server.getFiles();
	while (request.args.size() >= 2)
	{
		int clientFd = _server.getClientByNickname(request.args[0], _client);
		if (IrcHelper::clientExists(clientFd) == false)
		{
			_client->sendMessage(MessageHandler::ircNoSuchNick(_client->getNickname(), request.args[1]), NULL);
			return ;
		}
		if (non.find(request.args[1]) == non.end())
		{
			_client->sendMessage("DCC no file offered by " + request.args[0] + eol::IRC, NULL);
			return ;
		}
		File file(non[request.args[1]]);
		if (file.receiver != _client->getNickname() || file.sender != request.args[0])
		{
			_client->sendMessage("DCC no file offered by " + request.args[0] + eol::IRC, NULL);
			return ;
		}
		_clients[clientFd]->sendMessage(MessageHandler::msgSendingFile(request.args[1], _client->getNickname(), _client->getClientIp(), _client->getClientPort()), _client);
		std::fstream	ofs(request.args[1].c_str(), std::fstream::out);
		std::fstream	ifs(file.Path.c_str(), std::fstream::in);
		if (ofs.is_open())
			ofs << ifs.rdbuf();
		_client->sendMessage("DCC received file " + file.Name + " from " + file.sender + eol::IRC, NULL);
		_clients[clientFd]->sendMessage("DCC sent file " + file.Name + " for " + file.receiver + eol::IRC, _client);
		non.erase(request.args[1]);
		request.args.erase(request.args.begin()+1);
	}
}