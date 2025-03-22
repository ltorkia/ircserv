#include "../../incs/classes/Bot.hpp"

// === NAMESPACES ===
using namespace server_messages;

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
	std::cout << "🤖 BLA" << std::endl;
	struct sockaddr_in botAddr;
	int botSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (botSocketFd < 0)
	{
		perror(ERR_SOCKET_CREATION.c_str());
		return 1;
	}

	std::string serverIp = IrcHelper::getEnvValue("SERVER_IP");
	int port = atoi(IrcHelper::getEnvValue("PORT").c_str());
	if (serverIp.empty() || port == 0)
	{
		perror(ERR_ENV_VALUE.c_str());
		return 1;
	}

	botAddr.sin_family = AF_INET;
	botAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());
	botAddr.sin_port = htons(port);

	if (connect(botSocketFd, (struct sockaddr*)&botAddr, sizeof(botAddr)) < 0)
	{
		perror(ERR_SOCKET_CREATION.c_str());
		return 1;
	}

	std::cout << "🤖 Bot connecté au serveur !" << std::endl;

	// Envoyer immédiatement un message d'identification
	const char *helloBotMsg = "HELLO_BOT\r\n";
	if (send(botSocketFd, helloBotMsg, strlen(helloBotMsg), 0) < 0)
	{
		perror("Échec de l'envoi du message d'identification");
		close(botSocketFd);
		return 1;
	}
	return 0;
}