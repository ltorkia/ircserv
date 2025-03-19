#pragma once

#include <map>							// container map
#include <vector>						// container vector
#include <unistd.h>						// gestion fichiers -> chdir()
#include <fstream> 						// gestion fichiers -> std::ifstream, std::ofstream
#include <filesystem> 					// gestion fichiers -> std::filesystem

// === CLASSES ===
#include "Server.hpp"

// =========================================================================================

class File
{
	public:
		std::string Name;
		std::string Path;
		std::string sender;
		std::string receiver;

		File();
		File( std::string Name, std::string Path, std::string Sender, std::string Receiver );
		File( const File& x );
		~File();
		File &operator = (const File& src);

};


class Request
{
	public:
		std::vector<std::string>	args;
		std::string	command;

		Request(std::vector<std::string> arg, std::string cmd);
		~Request();
		Request( const Request& x );
		Request & operator = (const Request& src);

};