#include "../../../incs/server/Server.hpp"

// =========================================================================================

// === SERVER INFOS GETTERS ===

// ========================================= PUBLIC ========================================

/**
 * @brief Retrieves the file descriptor for the server socket.
 * 
 * This function returns the file descriptor associated with the server socket,
 * which is used for network communication.
 * 
 * @return int The file descriptor of the server socket.
 */
int Server::getServerSocketFd() const
{
	return _serverSocketFd;
}

/**
 * @brief Retrieves the local IP address of the server.
 * 
 * This function returns a constant reference to the local IP address
 * stored in the server instance.
 * 
 * @return const std::string& A constant reference to the local IP address.
 */
const std::string& Server::getLocalIP() const
{
	return _localIp;
}

/**
 * @brief Retrieves the port number the server is using.
 * 
 * @return int The port number.
 */
int Server::getPort() const
{
	return _port;
}

/**
 * @brief Retrieves the set of file descriptors for reading.
 * 
 * This function returns a copy of the file descriptor set that is monitored
 * for read events. It can be used to check which file descriptors are ready
 * for reading.
 * 
 * @return fd_set The set of file descriptors for reading.
 */
fd_set Server::getReadFds() const
{
	return _readFds;
}

/**
 * @brief Get the maximum file descriptor currently in use by the server.
 *
 * This function checks the highest file descriptor among the connected clients
 * and updates the server's maximum file descriptor (_maxFd) if a higher value
 * is found. It then returns the maximum file descriptor.
 *
 * @return int The maximum file descriptor currently in use by the server.
 */
int Server::getMaxFd()
{
	if (!_clients.empty())
	{
		int maxClientFd = _clients.rbegin()->first;
		if (maxClientFd > _maxFd)
			return maxClientFd;
	}
	return _serverSocketFd;
}

/**
 * @brief Returns the server's password.
 *
 * This function returns the server's password as a constant reference to a string.
 *
 * @return const std::string& The server's password.
 */
const std::string& Server::getServerPassword() const
{
	return _password;
}


// === CHANNELS ===

/**
 * @brief Returns the list of channels.
 *
 * This function returns the list of channels as a reference to a map of channel names to Channel pointers.
 *
 * @return std::map<std::string, Channel*>& The list of channels.
 */
std::map<std::string, Channel*>& Server::getChannels()
{
	return _channels;
}

/**
 * @brief Get the number of channels in the server.
 * 
 * This function returns the total count of channels currently managed by the server.
 * 
 * @return int The number of channels.
 */
int Server::getChannelCount() const
{
	return _channels.size();
}