#include "../../incs/bot/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/utils/Utils.hpp"

// === NAMESPACES ===
#include "../../incs/config/bot_config.hpp"
#include "../../incs/config/server_messages.hpp"
#include "../../incs/config/colors.hpp"

using namespace bot_config;
using namespace server_messages;
using namespace colors;

// =========================================================================================

/**
 * @file BotMain.cpp
 * @brief Entry point for the bot application.
 *
 * This file contains the main function that initializes and runs the bot.
 * It handles socket creation, server connection, and bot execution.
 *
 * @details
 * - The bot connects to a server using the IP address and port specified
 *   in environment variables.
 * - It uses a `Bot` object to manage its behavior and interactions.
 * - Errors during initialization or connection are caught and logged.
 *
 * @exception std::runtime_error Thrown when socket creation, environment
 * variable retrieval, or server connection fails.
 *
 * @return int Returns 0 on successful execution, or 1 if an error occurs.
 */
int main(void)
{
	try
	{
		struct sockaddr_in serverAddr;
		int botFd = socket(AF_INET, SOCK_STREAM, 0);
		if (botFd < 0)
			throw std::runtime_error(ERR_SOCKET_CREATION);

		Bot bot(botFd, BOTNICK, BOTUSER, BOTREAL);

		std::string serverIp = Utils::getEnvValue(env::SERVER_IP_KEY);
		int port = atoi(Utils::getEnvValue(env::SERVER_PORT_KEY).c_str());
		if (serverIp.empty() || port == 0)
			throw std::runtime_error(ERR_ENV_VALUE);

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());
		serverAddr.sin_port = htons(port);

		if (connect(botFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
			throw std::runtime_error(ERR_ECHEC_CONNECTION);

		std::cout << std::endl << BOT_CONNECTED << std::endl << std::endl;
		
		bot.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << RED << "❌ Error: " << RESET << e.what() << std::endl << std::endl;
		return 1;
	}
	return 0;
}