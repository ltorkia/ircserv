#include "../../../incs/server/FileData.hpp"
#include "../../../incs/server/CommandHandler.hpp"

// === OTHER CLASSES ===
#include "../../../incs/utils/Utils.hpp"
#include "../../../incs/utils/IrcHelper.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/irc_config.hpp"

using namespace file;

// =========================================================================================

// === FILE DATA STRUCT ===

// =========================================================================================

FileData::FileData() {}
FileData::FileData(const std::string& path, const std::string& sender, const std::string& receiver)
	: path(path), sender(sender), receiver(receiver) {}


// =========================================================================================

// === COMMAND HANDLER : MAIN FILE COMMANDS MANAGER ===

// ========================================= PRIVATE =======================================

/**
 * @brief Handles file-related commands such as sending or receiving files.
 * 
 * This function processes input commands to either send or retrieve files.
 * It validates the command format, extracts arguments, and performs the 
 * appropriate action based on the command type.
 * 
 * @throws std::runtime_error If the command is invalid, arguments are insufficient,
 *         or the HOME directory cannot be accessed.
 * 
 * The function performs the following steps:
 * - Tokenizes the input command and converts the command to uppercase.
 * - Validates the command type (must be SEND_CMD or GET_CMD).
 * - Extracts and validates the arguments from the command.
 * - Changes the current working directory to the user's HOME directory.
 * - Executes the appropriate file operation (_sendFile or _getFile) based on the command.
 */
void CommandHandler::_handleFile()
{
	std::vector<std::string> entry = Utils::getTokens(*_itInput, splitter::SENTENCE);
	Utils::toUpper(entry.front());

	if (entry.front() != SEND_CMD && entry.front() != GET_CMD)
		throw std::runtime_error(MessageBuilder::msgFileUsage(entry.front()));
	
	std::vector<std::string> args = Utils::getTokens(entry.back(), splitter::WORD);
	if (args.empty() || args.size() < 2)
		throw std::runtime_error(MessageBuilder::msgFileUsage(entry.front()));
	
	if (chdir(getenv("HOME")) != 0)
		throw std::runtime_error("HOME not found");
	
	entry.front() == SEND_CMD ? _sendFile(args) : _getFile(args);
}


// ==== SEND FILE ===

/**
 * @brief Handles the sending of files from one client to another using the DCC SEND protocol.
 * 
 * This function processes a list of file paths provided in the `args` vector and attempts to send
 * each file to the specified receiver. If any errors occur during the process (e.g., file not found,
 * receiver does not exist), appropriate error messages are sent back to the sender.
 * 
 * @param args A vector of strings where:
 *             - The first element is the nickname of the receiver.
 *             - The subsequent elements are file paths to be sent.
 * 
 * @throws std::invalid_argument If:
 *         - The number of arguments is less than 2.
 *         - The specified receiver does not exist.
 * 
 * The function performs the following steps:
 * 1. Validates the number of arguments and the existence of the receiver.
 * 2. Iterates through the list of file paths:
 *    - If a file cannot be opened, an error message is sent to the sender, and the file is skipped.
 *    - If the file is valid, it is added to the server's file list, and a DCC SEND request is sent
 *      to the receiver.
 * 3. Sends appropriate success or error messages to the sender and receiver.
 * 
 * @note This function assumes that the server and client objects are properly initialized and that
 *       the `_server` and `_client` members are valid.
 */
void CommandHandler::_sendFile(std::vector<std::string> args)
{
	size_t argsSize = args.size();
	if (argsSize < 2)
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), GET_CMD));
	
	std::string receiver = args[0];
	int clientFd = _server.getClientByNickname(receiver, _client);
	if (IrcHelper::clientExists(clientFd) == false)
		throw std::invalid_argument(MessageBuilder::ircNoSuchNick(_client->getNickname(), receiver));

	while (argsSize >= 2)
	{
		std::string path = args[1];
		std::fstream infile(path.c_str(), std::fstream::in);
		if (!infile)
		{
			_client->sendMessage(MessageBuilder::errorMsgSendFile(path), NULL);
			args.erase(args.begin() + 1);
			argsSize = args.size();
			continue;
		}

		std::string filename = _getFilename(path);
		_server.addFile(filename, path, _client->getNickname(), receiver);

		_client->sendMessage("DCC SEND request sent to " + receiver + ": " + filename + eol::IRC, NULL);
		_clients[clientFd]->sendMessage(MessageBuilder::msgSendFile(filename, _client->getNickname(), _client->getClientIp(), _client->getClientPort()), _client);
		
		args.erase(args.begin() + 1);
		argsSize = args.size();
	}
}


// ==== GET FILE ===

/**
 * @brief Handles the retrieval of a file sent via DCC (Direct Client-to-Client).
 * 
 * This function processes a request to retrieve a file from another client. It validates
 * the input arguments, checks the existence of the file, and ensures the sender and receiver
 * are correct. If all conditions are met, the file is transferred from the sender to the
 * receiver, and appropriate messages are sent to both clients.
 * 
 * @param args A vector of strings containing the command arguments. The first argument
 *             is the sender's nickname, and subsequent arguments are the names of the files
 *             to be retrieved.
 * 
 * @throws std::invalid_argument If the number of arguments is insufficient, the sender
 *                                does not exist, or the file does not meet the required
 *                                conditions (e.g., incorrect sender/receiver).
 * 
 * The function performs the following steps:
 * - Validates the number of arguments.
 * - Retrieves the sender's client file descriptor and checks if the sender exists.
 * - Iterates through the list of requested files:
 *   - Checks if the file exists and is offered by the correct sender to the correct receiver.
 *   - Opens the file for reading and creates a new file for writing.
 *   - Copies the file content line by line.
 *   - Sends confirmation messages to both the sender and receiver.
 *   - Removes the file from the server's file list after successful transfer.
 *   - Handles errors such as file not found, inability to open the file, or write errors.
 */
void CommandHandler::_getFile(std::vector<std::string> args)
{
	size_t argsSize = args.size();
	if (argsSize < 2)
		throw std::invalid_argument(MessageBuilder::ircNeedMoreParams(_client->getNickname(), GET_CMD));

	std::string sender = args[0];
	int clientFd = _server.getClientByNickname(sender, _client);
	if (IrcHelper::clientExists(clientFd) == false)
		throw std::invalid_argument(MessageBuilder::ircNoSuchNick(_client->getNickname(), args[1]));

	while (argsSize >= 2)
	{
		std::string fileName = args[1];
		std::map<std::string, FileData> files = _server.getFiles();
		std::map<std::string, FileData>::iterator it = files.find(fileName);
		if (it == files.end())
		{
			_client->sendMessage("DCC no file offered by " + sender + eol::IRC, NULL);
			args.erase(args.begin() + 1);
			argsSize = args.size();
			continue;
		}

		FileData file = it->second;
		if (file.sender != sender || file.receiver != _client->getNickname() || sender == _client->getNickname())
			throw std::invalid_argument(MessageBuilder::ircNoSuchNick(_client->getNickname(), sender));

		std::fstream infile(file.path.c_str(), std::fstream::in);
		if (!infile)
		{
			_client->sendMessage("DCC can't open file " + file.path + eol::IRC, NULL);
			args.erase(args.begin() + 1);
			argsSize = args.size();
			continue;
		}

		std::fstream outfile(fileName.c_str(), std::fstream::out);
		if (!outfile)
		{
			_client->sendMessage("DCC can't write file " + file.path + eol::IRC, NULL);
			infile.close();
			args.erase(args.begin() + 1);
			argsSize = args.size();
			continue;
		}

		// Copie ligne par ligne
		std::string line;
		while (std::getline(infile, line))
			outfile << line << std::endl;

		// Fermeture explicite des fichiers
		infile.close();
		outfile.close();
		
		_client->sendMessage("DCC received file " + fileName + " from " + file.sender + eol::IRC, NULL);
		_clients[clientFd]->sendMessage("DCC sent file " + fileName + " for " + file.receiver + eol::IRC, _client);

		// Suppression du fichier
		_server.removeFile(it->first);

		args.erase(args.begin() + 1);
		argsSize = args.size();
	}
}

/**
 * @brief Extracts the filename from a given file path.
 * 
 * This function takes a file path as input and returns the filename
 * by finding the last occurrence of the '/' character and extracting
 * the substring that follows it.
 * 
 * @param path The full file path as a string.
 * @return A string containing the filename extracted from the path.
 */
std::string CommandHandler::_getFilename(const std::string& path) const
{
	size_t pos = path.find_last_of('/');
	return path.substr(pos + 1);
}