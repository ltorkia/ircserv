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
		File( std::string name, std::string sender, std::string receiver );
		~File();
		File(const File& x);
		File &operator = (const File& src);

		// === GETTERS ===
		std::string getFileName() const;
		std::string getSender() const;
		std::string getReceiver() const;

	private:
		std::string _name, _sender, _receiver;
};