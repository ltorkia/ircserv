#pragma once

#include <map>							// container map
#include <vector>						// container vector
#include <unistd.h>						// gestion fichiers -> chdir()
#include <fstream> 						// gestion fichiers -> std::ifstream, std::ofstream
#include <filesystem> 					// gestion fichiers -> std::filesystem

// =========================================================================================

class File
{
	public:
		File();
		File( std::string name, std::string path, std::string sender, std::string receiver );
		File( const File& x );
		~File();
		File &operator = (const File& src);

		// === GETTERS ===
		std::string getFileName() const;
		std::string getPath() const;
		std::string getSender() const;
		std::string getReceiver() const;

	private:
		std::string _name, _path, _sender, _receiver;

};


class Request
{
	public:
		Request(std::vector<std::string> arg, std::string cmd);
		~Request();
		Request( const Request& x );
		Request & operator = (const Request& src);

		// === GETTERS ===
		size_t getArgsSize() const;
		std::vector<std::string> getArgs() const;
		std::string getCommand() const;

	private:
		std::vector<std::string> _args;
		std::string	_command;

};