/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command_File.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 09:25:46 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileData.hpp"
#include "Command.hpp"

// === OTHER CLASSES ===
#include "Utils.hpp"
#include "IrcHelper.hpp"
#include "MessageBuilder.hpp"

// === NAMESPACES ===
#include "irc_config.hpp"
#include "server_messages.hpp"

using namespace file;
using namespace server_messages;

/**
 * @note In irssi, simply typing 'DCC' triggers the automatic implementation of peer-to-peer file sharing using the DCC SEND protocol.
 * For Netcat, however, we chose a more straightforward approach: local file transfer via environment variables, rather than a full DCC implementation. 
 * This allows basic file transmission functionality but does not adhere strictly to the DCC protocol as defined for IRC clients.
**/

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
 * This function processes input commands to either send or receive files.
 * It validates the command format, ensures the required arguments are present,
 * and changes the working directory to the user's home directory before
 * delegating the operation to the appropriate handler.
 * 
 * @throws std::invalid_argument If the command is not SEND_CMD or GET_CMD,
 *                                or if the arguments are missing or insufficient.
 * @throws std::runtime_error If the HOME environment variable is not found or
 *                            the directory change fails.
 */
void Command::_handleFile()
{
	std::vector<std::string> entry = Utils::getTokens(*_itInput, splitter::SENTENCE);
	Utils::toUpper(entry.front());

	if (entry.front() != SEND_CMD && entry.front() != GET_CMD)
		throw std::invalid_argument(MessageBuilder::msgFileUsage(entry.front()));
	
	std::vector<std::string> args = Utils::getTokens(entry.back(), splitter::WORD);
	if (args.empty() || args.size() < 2)
		throw std::invalid_argument(MessageBuilder::msgFileUsage(entry.front()));
	
	if (chdir(getenv("HOME")) != 0)
		throw std::runtime_error(ERR_HOME_NOT_FOUND);
	
	entry.front() == SEND_CMD ? _sendFile(args) : _getFile(args);
}


// ==== SEND FILE ===

/**
 * @brief Sends a file based on the provided arguments.
 * 
 * This function handles the process of sending a file from the client to another client.
 * It performs the following steps:
 * 1. Retrieves the receiver's information using the provided nickname.
 * 2. Opens the specified file to be sent.
 * 3. If the file opens successfully, it registers the file for transfer and sends a notification to both the sender and the receiver.
 * 4. If any error occurs during file opening, an error message is sent to the client and the process continues with the next argument.
 * 
 * @param args A vector of strings containing the arguments required for the file transfer.
 *             The first element is the receiver's nickname, and the second element is the file path.
 */
void Command::_sendFile(std::vector<std::string>& args)
{
	size_t argsSize = args.size();
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
			_handleFileError(args, argsSize, MessageBuilder::errorMsgOpenFile(path));
			continue;
		}

		std::string filename = _getFilename(path);
		_server.addFile(filename, path, _client->getNickname(), receiver);

		_client->sendMessage(MessageBuilder::msgRequestSent(filename, receiver), NULL);
		_clients[clientFd]->sendMessage(MessageBuilder::msgSendFile(filename, _client->getNickname(), _client->getClientIp(), _client->getClientPort()), _client);
		
		args.erase(args.begin() + 1);
		argsSize = args.size();
	}
}


// ==== GET FILE ===

/**
 * @brief Handles the retrieval of a file based on the provided arguments.
 * 
 * This function handles the process of retrieving a file sent by another client. The process includes:
 * 1. Verifying the sender's nickname and ensuring the file exists.
 * 2. Checking if the sender and receiver match the expected roles.
 * 3. Opening the file for reading and writing.
 * 4. If any errors occur during the file handling, such as file not found or failure to open the file, an error message is sent to the client.
 * 5. If the file is successfully retrieved, it is copied from the source file to the target file.
 * 6. After completion, notifications are sent to both the sender and receiver, and the file is removed from the server's file list.
 * 
 * @param args A vector of strings containing the arguments for the file retrieval command.
 *             The first element is the sender's nickname, and the second element is the filename.
 */
void Command::_getFile(std::vector<std::string>& args)
{
	size_t argsSize = args.size();
	std::string sender = args[0];
	int clientFd = _server.getClientByNickname(sender, _client);
	if (IrcHelper::clientExists(clientFd) == false)
		throw std::invalid_argument(MessageBuilder::ircNoSuchNick(_client->getNickname(), sender));

	while (argsSize >= 2)
	{
		std::string filename = args[1];
		std::map<std::string, FileData> files = _server.getFiles();
		std::map<std::string, FileData>::iterator it = files.find(filename);
		if (it == files.end())
		{
			_handleFileError(args, argsSize, MessageBuilder::errorMsgNoFile(sender));
			continue;
		}

		FileData file = it->second;
		if (file.sender != sender || file.receiver != _client->getNickname() || sender == _client->getNickname())
			throw std::invalid_argument(MessageBuilder::ircNoSuchNick(_client->getNickname(), sender));

		std::fstream infile(file.path.c_str(), std::fstream::in);
		if (!infile)
		{
			_handleFileError(args, argsSize, MessageBuilder::errorMsgOpenFile(file.path));
			continue;
		}

		std::fstream outfile(filename.c_str(), std::fstream::out);
		if (!outfile)
		{
			_handleFileError(args, argsSize, MessageBuilder::errorMsgWriteFile(file.path));
			infile.close();
			continue;
		}

		// Copie ligne par ligne
		std::string line;
		while (std::getline(infile, line))
			outfile << line << std::endl;

		// Fermeture explicite des fichiers
		infile.close();
		outfile.close();
		
		_client->sendMessage(MessageBuilder::msgFileReceived(filename, file.sender), NULL);
		_clients[clientFd]->sendMessage(MessageBuilder::msgFileSent(filename, file.receiver), _client);

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
std::string Command::_getFilename(const std::string& path) const
{
	size_t pos = path.find_last_of('/');
	return path.substr(pos + 1);
}

/**
 * @brief Handles file-related errors during command execution.
 * 
 * This function sends an error message to the client and adjusts the arguments
 * list by removing the problematic argument. It also updates the size of the
 * arguments list accordingly.
 * 
 * @param args A reference to the vector of arguments passed to the command.
 * @param argsSize A reference to the size of the arguments vector, which will
 *                 be updated after modifying the arguments list.
 * @param errorMessage The error message to be sent to the client.
 */
void Command::_handleFileError(std::vector<std::string>& args, size_t& argsSize, const std::string& errorMessage)
{
	_client->sendMessage(errorMessage, NULL);
	args.erase(args.begin() + 1);
	argsSize = args.size();
}