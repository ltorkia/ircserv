#include "../../../incs/server/CommandHandler.hpp"
#include "../../../incs/server/CommandHandler_File.hpp"

// === OTHER CLASSES ===
#include "../../../incs/utils/Utils.hpp"
#include "../../../incs/utils/IrcHelper.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/irc_config.hpp"

using namespace file_cmd;

// =========================================================================================

// === FILE CLASS ===

// ========================================= PUBLIC ========================================

File::File() {}
File::File(std::string name, std::string sender, std::string receiver ): _name(name), _sender(sender), _receiver(receiver) {}
File::~File() {}
File::File(const File &x) {*this = x;}
File & File::operator=(const File &src)
{
	if (this == &src)
		return *this;
	this->_name = src._name;
	this->_sender = src._sender;
	this->_receiver = src._receiver;
	return *this;
};

// === GETTERS ===

std::string File::getFileName() const {return _name;}
std::string File::getSender() const {return _sender;}
std::string File::getReceiver() const {return _receiver;}


// =========================================================================================

// === COMMAND HANDLER : MAIN FILE COMMANDS MANAGER ===

// ========================================= PRIVATE =======================================

void CommandHandler::_handleFile()
{
	std::vector<std::string> entry = Utils::getTokens(*_itInput, splitter::SENTENCE);
	Utils::toUpper(entry.front());
	if (entry.front() != SEND_CMD && entry.front() != GET_CMD)
		throw std::runtime_error(MessageBuilder::ircNeedMoreParams(_client->getNickname(), entry.front()));
	
	std::vector<std::string> args = Utils::getTokens(entry.back(), splitter::WORD);
	if (args.empty() || args.size() < 2)
		throw std::runtime_error(MessageBuilder::msgFileUsage(entry.front()));
	
	if (chdir(getenv("HOME")) != 0)
		throw std::runtime_error("HOME not found");
	
	entry.front() == SEND_CMD ? _sendFile(args) : _sendFile(args);
}


// ==== SEND FILE ===

void CommandHandler::_sendFile(std::vector<std::string> args)
{
	size_t argsSize = _getArgsSize();
	if (argsSize < 2)
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), GET_CMD));
	
	std::string receiver = args[0];
	int clientFd = _server.getClientByNickname(receiver, _client);
	if (IrcHelper::clientExists(clientFd) == false)
		throw std::invalid_argument(MessageBuilder::ircNoSuchNick(_client->getNickname(), receiver));

	while (argsSize >= 2)
	{
		std::string path = args[1];
		std::fstream ifs(path.c_str(), std::fstream::in);
		if (ifs.fail())
		{
			_client->sendMessage(MessageBuilder::errorMsgSendFile(path), NULL);
			continue;
		}
		std::string filename = _getFilename(path);
		File file(filename, _client->getNickname(), receiver);
		_files.insert(std::pair<std::string, File>(filename, file));

		_client->sendMessage("DCC SEND request sent to " + receiver + ": " + filename + eol::IRC, NULL);
		_clients[clientFd]->sendMessage(MessageBuilder::msgSendFile(filename, _client->getNickname(), _client->getClientIp(), _client->getClientPort()), _client);
		args.erase(args.begin() + 1);
	}
}


// ==== GET FILE ===

void	CommandHandler::_getFile(std::vector<std::string> args)
{
	size_t argsSize = _getArgsSize();
	if (argsSize < 2)
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), GET_CMD));

	std::string sender = args[0];
	int clientFd = _server.getClientByNickname(sender, _client);
	if (IrcHelper::clientExists(clientFd) == false)
		throw std::invalid_argument(MessageBuilder::ircNoSuchNick(_client->getNickname(), args[1]));

	while (argsSize >= 2)
	{
		std::string path = args[1];
		if (_files.find(path) == _files.end())
		{
			_client->sendMessage("DCC no file offered by " + sender + eol::IRC, NULL);
			return ;
		}
		File file(_files[path]);
		std::string fileName = file.getFileName();
		std::string fileSender = file.getSender();
		std::string fileReceiver = file.getReceiver();

		if (fileSender != sender || fileReceiver != _client->getNickname() || sender == _client->getNickname())
			throw std::invalid_argument(MessageBuilder::ircNoSuchNick(_client->getNickname(), sender));

		_clients[clientFd]->sendMessage(MessageBuilder::msgSendingFile(path, _client->getNickname(), _client->getClientIp(), _client->getClientPort()), _client);
		std::fstream ofs(path.c_str(), std::fstream::out);
		std::fstream ifs(path.c_str(), std::fstream::in);
		if (ofs.is_open())
			ofs << ifs.rdbuf();
		_client->sendMessage("DCC received file " + fileName + " from " + fileSender + eol::IRC, NULL);
		_clients[clientFd]->sendMessage("DCC sent file " + fileName + " for " + fileReceiver + eol::IRC, _client);
		_files.erase(path);
		args.erase(args.begin() + 1);
	}
}

std::string CommandHandler::_getFilename(const std::string& path) const
{
	size_t pos = path.find_last_of('/');
	return path.substr(pos + 1);
}

size_t CommandHandler::_getArgsSize() const
{
	return _vectorInput.size();
}