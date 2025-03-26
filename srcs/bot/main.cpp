#include "../../incs/classes/bot/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/utils/IrcHelper.hpp"

// === NAMESPACES ===
#include "../../incs/config/bot.hpp"
#include "../../incs/config/server_messages.hpp"
#include "../../incs/config/colors.hpp"

using namespace server_messages;
using namespace colors;

// =========================================================================================

/**
 * @brief Entry point of the bot application.
 * 
 * This function initializes the bot, sets up the server connection, and runs the bot.
 * It handles exceptions and prints error messages if any occur during execution.
 * 
 * @return int Returns 0 on successful execution, 1 on error.
 * 
 * @throws std::runtime_error If socket creation fails, environment variables are invalid,
 *                            or connection to the server fails.
 */
int main(void)
{
	try
	{
		struct sockaddr_in serverAddr;
		int botFd = socket(AF_INET, SOCK_STREAM, 0);
		if (botFd < 0)
			throw std::runtime_error(ERR_SOCKET_CREATION);

		Bot bot(botFd, bot::NICK, bot::USER, bot::REALNAME);

		std::string serverIp = IrcHelper::getEnvValue(env::SERVER_IP_KEY);
		int port = atoi(IrcHelper::getEnvValue(env::SERVER_PORT_KEY).c_str());
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