#include "../../incs/classes/bot/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/utils/IrcHelper.hpp"
#include "../../incs/classes/utils/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/bot.hpp"
#include "../../incs/config/commands.hpp"
#include "../../incs/config/server_messages.hpp"

using namespace server_messages;

// =========================================================================================

// === AUTHENTICATE ===
/**
 * @brief Authenticates the bot based on the received message.
 *
 * This function checks if the received message matches the expected welcome
 * message. If it does, the bot is marked as authenticated. If the bot has not
 * yet sent authentication information, it sends the necessary authentication
 * details.
 *
 * @param message The message received that needs to be checked for authentication.
 */
void Bot::_authenticate(const std::string& message)
{
	std::string welcomeMsg = MessageHandler::ircWelcomeMessage(_botNick, _botMask);
	if (message == welcomeMsg)
	{
		_isAuthenticated = true;
		std::cout << BOT_AUTHENTICATED << std::endl << std::endl;
	}
	if (_hasSentAuthInfos == false)
		_sendAuthInfos();
}

/**
 * @brief Authenticates the bot with the IRC server.
 *
 * This function performs the authentication process for the bot by sending
 * the necessary IRC commands to the server. It sends the server password,
 * bot nickname, and bot username in sequence, with a delay between each
 * command to ensure proper processing by the server.
 *
 * The function checks if the bot is already authenticated before proceeding
 * with the authentication steps. If the bot is not authenticated, it sends
 * the following commands:
 * - PASS: Sends the server password.
 * - NICK: Sends the bot's nickname.
 * - USER: Sends the bot's username and real name.
 *
 * The function also prints messages to the console to indicate the progress
 * of the authentication process.
 */
void Bot::_sendAuthInfos()
{
	std::string pass = IrcHelper::getEnvValue(env::PASS_KEY);
	if (pass.empty())
		throw std::invalid_argument(ERR_ENV_VALUE);

	const std::string BOTPASS_CMD = commands::PASS + " " + pass;

	_sendMessage(BOTPASS_CMD);
	std::cout << MSG_SENDING_PASSWORD << std::endl << std::endl; 
	sleep(1);

	_sendMessage(BOTNICK_CMD);
	std::cout << MSG_SENDING_NICKNAME << std::endl << std::endl; 
	sleep(1);

	_sendMessage(BOTUSER_CMD);
	std::cout << MSG_SENDING_USERNAME << std::endl << std::endl; 
	sleep(1);

	_hasSentAuthInfos = true;
}