#include "../../../incs/server/Server.hpp"

// === OTHER CLASSES ===
#include "../../../incs/utils/IrcHelper.hpp"
#include "../../../incs/utils/MessageBuilder.hpp"

// === NAMESPACES ===
#include "../../../incs/config/irc_config.hpp"
#include "../../../incs/config/server_messages.hpp"

using namespace server_messages;

// =========================================================================================

// === SERVER SETTINGS ===

// ========================================= PUBLIC ========================================

// === STATIC SIGNAL VARIABLE + SIGNAL HANDLER ===

/**
 * @brief A static volatile variable to store the signal received status.
 * 
 * This variable is used to indicate whether a signal has been received.
 * It is declared as volatile to prevent the compiler from optimizing
 * out accesses to it, as it may be modified asynchronously by a signal
 * handler.
 */
volatile sig_atomic_t Server::signalReceived = boolean::FALSE;

/**
 * @brief Signal handler for the server.
 *
 * This function handles the SIGINT and SIGTSTP signals, which are typically
 * generated by pressing Ctrl+C or Ctrl+Z, respectively. When these signals are
 * caught, the server sets signalReceived to true to clean and terminate the program.
 *
 * @param signal The signal number that was caught.
 *
 * @return void
 */
void Server::signalHandler(int signal)
{
	const char* signalType;

	switch (signal)
	{
		case SIGINT: signalType = "SIGINT"; break;
		case SIGTSTP: signalType = "SIGTSTP"; break;
		default: signalType = "Unknown";
	}

	std::cout << MessageBuilder::msgSignalCaught(signalType) << std::endl;
	signalReceived = boolean::TRUE;
}


// === CONSTUCTOR / DESTRUCTOR ===

/**
 * @brief Constructor for the Server class.
 *
 * Initializes the server with the given port and password.
 * Sets the server socket file descriptor and maximum file descriptor to -1 and 0, respectively.
 *
 * @param port The port number for the server to listen on. It must be a valid port number (0-65535).
 * @param password The password required for clients to connect to the server. It must be a non-empty string.
 *
 * @throws std::invalid_argument If the port number is not within the valid range or if the password is invalid or empty.
*/
Server::Server(const std::string &port, const std::string &password)
	: _serverSocketFd(-1), _maxFd(0)
{
	_port = IrcHelper::validatePort(port);

	if (!IrcHelper::isValidPassword(password, true))
		throw std::invalid_argument(ERR_INVALID_PASSWORD);
	_password = password;

	_init();
}

/**
 * @brief Destructor for the Server class.
 *
 * This destructor is responsible for cleaning up resources
 * used by the Server instance. It calls the private _clean()
 * method to handle the cleanup process.
 */
Server::~Server()
{
	_clean();
}


// === LAUNCH SERVER ===

/**
 * @brief Launches the IRC server.
 *
 * This function starts the IRC server by calling the start() function.
 * It catches any exceptions thrown by the start() function and prints an error message.
 *
 * @return void
 */
void Server::launch()
{
	try
	{
		_start();
	}
	catch (const std::exception &e)
	{
		std::cerr << MessageBuilder::msgServerException(e) << std::endl;
	}
}


// ========================================= PRIVATE =======================================

Server::Server() {}
Server::Server(const Server& src) {(void) src;}
Server & Server::operator=(const Server& src) {(void) src; return *this;}