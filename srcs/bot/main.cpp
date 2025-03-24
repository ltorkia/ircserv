#include "../../incs/classes/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/IrcHelper.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/server_messages.hpp"
#include "../../incs/config/colors.hpp"

using namespace server_messages;
using namespace colors;

// =========================================================================================

/**
 * @brief Sets the signal handler for SIGINT and SIGTSTP signals.
 *
 * This function sets the signal handler for SIGINT and SIGTSTP signals to the `signalHandler` function.
 * The signal handler function is responsible for handling the termination of the server gracefully.
 *
 * @return void
 *
 * @throws std::runtime_error If an error occurs while setting the signal handler.
 *
 * @see signalHandler
 */
static void setSignal()
{
	struct sigaction sa;
	sa.sa_handler = &Bot::signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTSTP, &sa, NULL) == -1)
		throw std::runtime_error(ERR_SET_SIGNAL);
}

/**
 * @brief Entry point of the bot application.
 * 
 * This function initializes a socket, retrieves server IP and port from environment variables,
 * connects to the server, and sends an identification message.
 * 
 * @return int Returns 0 on successful execution, 1 on error.
 * 
 * Error codes:
 * - 1: Socket creation failed.
 * - 1: Environment variable retrieval failed.
 * - 1: Connection to the server failed.
 * - 1: Sending identification message failed.
 */
int main(void)
{
	int botFd = -1;
	bool error = false;

	try
	{
		setSignal();

		struct sockaddr_in serverAddr;
		botFd = socket(AF_INET, SOCK_STREAM, 0);
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

		std::cout << "🤖 Bot connecté au serveur !" << std::endl;

		bot.listenActivity();
	}
	catch (const std::exception &e)
	{
		error = true;
		std::cerr << RED << "Error: " << RESET << e.what() << std::endl << std::endl;
	}

	if (botFd != -1)
		close(botFd);
	
	return error ? 1 : 0;
}