#pragma once

#include <string>

struct FileData
{
	std::string path, sender, receiver;

	FileData();
	FileData(const std::string& path, const std::string& sender, const std::string& receiver);
};