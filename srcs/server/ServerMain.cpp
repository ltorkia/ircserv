#include "../../incs/server/Server.hpp"

// === NAMESPACES ===
#include "../../incs/config/server_messages.hpp"
#include "../../incs/config/colors.hpp"

using namespace server_messages;
using namespace colors;

// =========================================================================================

/**
 * @file ServerMain.cpp
 * @brief Entry point for the IRC server application.
 *
 * This file contains the main function which initializes and launches the server.
 * It validates the command-line arguments, creates a Server instance, and starts
 * the server. Any exceptions encountered during execution are caught and logged.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 *             - argv[1]: The port number on which the server will listen.
 *             - argv[2]: The password required for client connections.
 *
 * @return Returns 0 on successful execution, or 1 if an error occurs.
 *
 * @note The program expects exactly two arguments: port and password.
 *       If the arguments are invalid or an exception is thrown, an error
 *       message is displayed, and the program exits with a non-zero status.
 */
int main(int argc, char **argv)
{

	if (argc != 3)
	{
		std::cout << RED << "❌ Usage: " << RESET << ERR_INVALID_ARGUMENTS << std::endl << std::endl;
		return 1;
	}

	try
	{
		std::string port(argv[1]);
		std::string password(argv[2]);
		
		Server server(port, password);
		server.launch();

	} 
	catch (const std::exception &e)
	{
		std::cerr << RED << "❌ Error: " << RESET << e.what() << std::endl << std::endl;
		return 1;
	}
	
	return 0;
}