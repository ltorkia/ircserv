#include "../../incs/classes/commands/CommandHandler.hpp"
#include "../../incs/classes/commands/CommandHandler_File.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/utils/Utils.hpp"
#include "../../incs/classes/utils/IrcHelper.hpp"
#include "../../incs/classes/utils/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/server_messages.hpp"

using namespace file_cmd;

//---------------------------------------------------FILE METHODS---------------------------------------------------//

File::File() {}
File::File(std::string name, std::string path, std::string sender, std::string receiver ): _name(name), _path(path), _sender(sender), _receiver(receiver) {}
File::File( const File &x ) {*this = x;}
File::~File() {}
File & File::operator=( const File &src )
{
	if (this == &src)
		return *this;
	this->_name = src._name;
	this->_path = src._path;
	this->_sender = src._sender;
	this->_receiver = src._receiver;
	return *this;
};

// === GETTERS ===
std::string File::getFileName() const {return _name;}
std::string File::getPath() const {return _path;}
std::string File::getSender() const {return _sender;}
std::string File::getReceiver() const {return _receiver;}

//---------------------------------------------------REQUEST METHODS---------------------------------------------------//

Request::Request(std::vector<std::string> arg, std::string cmd): _args(arg), _command(cmd) {};
Request::Request(const Request& x ) {*this = x;};
Request::~Request() {};
Request& Request::operator=(const Request& src)
{
	if (this == &src)
		return *this;
	this->_args = src._args;
	this->_command = src._command;
	return *this;
};

// === GETTERS ===
size_t Request::getArgsSize() const { return _args.size(); }
std::vector<std::string> Request::getArgs() const { return _args; }
std::string Request::getCommand() const { return _command; }


//---------------------------------------------------DISTRIBUTION METHOD---------------------------------------------------//

void CommandHandler::_handleFile()
{
	std::vector<std::string> entry = Utils::getTokens(*_itInput, splitter::SENTENCE);
	if (entry.front() == SEND_CMD)
		_sendFile(entry);
	else if (entry.front() == GET_CMD)
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
		_client->sendMessage(MessageHandler::ircNeedMoreParams(_client->getNickname(), SEND_CMD), NULL);
		return ;
	}
	Request	request(req, SEND_CMD);
	size_t argsSize = request.getArgsSize();
	std::vector<std::string> args = request.getArgs();

	if (argsSize < 2)
	{
		_client->sendMessage(MessageHandler::ircNeedMoreParams(_client->getNickname(), SEND_CMD), NULL);
		return ;
	}

	while (argsSize >= 2)
	{
		int clientFd = _server.getClientByNickname(args[0], _client);
		if (IrcHelper::clientExists(clientFd) == false)
		{
			_client->sendMessage(MessageHandler::ircNoSuchNick(_client->getNickname(), args[1]), NULL);
			return ;
		}
		std::fstream ifs(args[1].c_str(), std::fstream::in);
		if (ifs.fail())
		{
			_client->sendMessage(MessageHandler::errorMsgSendFile(args[1]), NULL);
			return ;
		}
		size_t pos = args[1].find_last_of('/');
		std::string filename = args[1].substr(pos + 1);
		File file(filename, args[1], _client->getNickname(), args[0]);
		_server.getFiles().insert(std::pair<std::string, File>(filename, file));
		_client->sendMessage("DCC SEND request sent to " + args[0] + ": " + filename + eol::IRC, NULL);
		_clients[clientFd]->sendMessage(MessageHandler::msgSendFile(filename, _client->getNickname(), _client->getClientIp(), _client->getClientPort()), _client);
		args.erase(args.begin() +1 );
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
		_client->sendMessage(MessageHandler::ircNeedMoreParams(_client->getNickname(), GET_CMD), NULL);
		return ;
	}
	Request	request(req, GET_CMD);
	size_t argsSize = request.getArgsSize();
	std::vector<std::string> args = request.getArgs();
	if (argsSize < 2)
	{
		_client->sendMessage(MessageHandler::ircNeedMoreParams(_client->getNickname(), GET_CMD), NULL);
		return ;
	}
	std::map<std::string, File> non = _server.getFiles();
	while (argsSize >= 2)
	{
		int clientFd = _server.getClientByNickname(args[0], _client);
		if (IrcHelper::clientExists(clientFd) == false)
		{
			_client->sendMessage(MessageHandler::ircNoSuchNick(_client->getNickname(), args[1]), NULL);
			return ;
		}
		if (non.find(args[1]) == non.end())
		{
			_client->sendMessage("DCC no file offered by " + args[0] + eol::IRC, NULL);
			return ;
		}
		File file(non[args[1]]);
		std::string fileName = file.getFileName();
		std::string path = file.getPath();
		std::string sender = file.getSender();
		std::string receiver = file.getReceiver();

		if (receiver != _client->getNickname() || sender != args[0])
		{
			_client->sendMessage("DCC no file offered by " + args[0] + eol::IRC, NULL);
			return ;
		}
		_clients[clientFd]->sendMessage(MessageHandler::msgSendingFile(args[1], _client->getNickname(), _client->getClientIp(), _client->getClientPort()), _client);
		std::fstream	ofs(args[1].c_str(), std::fstream::out);
		std::fstream	ifs(path.c_str(), std::fstream::in);
		if (ofs.is_open())
			ofs << ifs.rdbuf();
		_client->sendMessage("DCC received file " + fileName + " from " + sender + eol::IRC, NULL);
		_clients[clientFd]->sendMessage("DCC sent file " + fileName + " for " + receiver + eol::IRC, _client);
		non.erase(args[1]);
		args.erase(args.begin() + 1);
	}
}