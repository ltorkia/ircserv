#include "../../incs/classes/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/classes/IrcHelper.hpp"
#include "../../incs/classes/MessageHandler.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/commands.hpp"

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
		std::cout << "Welcome message received, bot is authenticated." << std::endl;
		_isAuthenticated = true;
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
	std::cout << "\nAuthenticating bot with the server..." << std::endl;

	std::string pass = IrcHelper::getEnvValue(env::PASS_KEY);
	if (pass.empty())
	{
		std::cerr << "Erreur : mot de passe IRC non trouvé dans les variables d'environnement." << std::endl;
		exit(1);
	}

	// connection to irc server
	_sendMessage(commands::PASS + " " + pass);
	std::cout << "\nSending password, waiting for authentication..." << std::endl; 
	sleep(1);

	_sendMessage(commands::NICK + " " + _botNick);
	std::cout << "\nSending nickname, waiting for second authentication..." << std::endl; 
	sleep(1);

	_sendMessage(commands::USER + " " + _botUser + " 0 * :" + _botReal);
	std::cout << "\nSending username, getting bored..." << std::endl; 
	sleep(1);

	_hasSentAuthInfos = true;
}