#include "../../incs/bot/Bot.hpp"

// === OTHER CLASSES ===
#include "../../incs/utils/Utils.hpp"

// === NAMESPACES ===
#include "../../incs/config/irc_config.hpp"
#include "../../incs/config/bot_config.hpp"
#include "../../incs/config/server_messages.hpp"
#include "../../incs/config/colors.hpp"

using namespace bot_config;
using namespace server_messages;
using namespace colors;

// =========================================================================================

// === STATIC SIGNAL VARIABLE + SIGNAL HANDLER ===

/**
 * @brief A static volatile variable to store the signal received status.
 * 
 * This variable is used to indicate whether a signal has been received.
 * It is declared as volatile to prevent the compiler from optimizing
 * out accesses to it, as it may be modified asynchronously by a signal
 * handler.
 */
volatile sig_atomic_t Bot::signalReceived = boolean::FALSE;

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
void Bot::signalHandler(int signal)
{
	const char* signalType;

	switch (signal)
	{
		case SIGINT: signalType = "SIGINT"; break;
		case SIGTSTP: signalType = "SIGTSTP"; break;
		default: signalType = "Unknown";
	}

	std::cout << " " << signalType << " " << COLOR_ERR << BOT_DISCONNECTED << RESET << std::endl;
	signalReceived = boolean::TRUE;
}

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
void Bot::setSignal()
{
	struct sigaction sa;
	sa.sa_handler = &Bot::signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTSTP, &sa, NULL) == -1)
		throw std::runtime_error(ERR_SET_SIGNAL);
}


// === CONSTRUCTOR / DESTRUCTOR ===

Bot::Bot(int botFd, const std::string& nick, const std::string& user, const std::string& real)
	: _hasSentAuthInfos(false), _isAuthenticated(false), _botFd(botFd),
	_botNick(nick), _botUser(user), _botReal(real), _botMask(_botNick + "!" + '~' + _botUser + "@" + Utils::getEnvValue(env::SERVER_IP_KEY))
{
	setSignal();
}
Bot::~Bot()
{
	if (_botFd != -1)
		close(_botFd);
}


// === LISTEN ACTIVITY ===

/**
 * @brief Runs the bot's main loop, handling incoming messages and signals.
 * 
 * This function continuously monitors the bot's file descriptor for incoming
 * messages using the select() system call. It breaks out of the loop if a 
 * termination signal is received. The function sets up a timeout for the 
 * select() call to periodically check for new messages and handle them 
 * appropriately.
 * 
 * @note The function uses a timeout of 500 milliseconds for the select() call.
 * 
 * @details
 * - Initializes the file descriptor set.
 * - Enters an infinite loop to monitor the bot's file descriptor.
 * - Resets internal information.
 * - Breaks the loop if a termination signal is received.
 * - Uses select() to wait for activity on the file descriptor.
 * - If activity is detected, calls _handleMessage() to process the message.
 * - Clears the file descriptor set before exiting.
 */
void Bot::run()
{
	fd_set readFds;
	FD_ZERO(&readFds);
	while (true)
	{
		_resetInfos();
		if (signalReceived)
			break;
			
		FD_SET(_botFd, &readFds);
		struct timeval timeout = {0, 500000};
		
		int activity = select(_botFd + 1, &readFds, NULL, NULL, &timeout);
		if (activity > 0 && FD_ISSET(_botFd, &readFds))
			_handleMessage();
	}
	FD_CLR(_botFd, &readFds);
}

/**
 * @brief Resets the internal state of the Bot object.
 *
 * This function clears all the internal data members of the Bot object,
 * including target, client nickname, channel name, input, command, age argument,
 * and resets the command position and date-related members to their initial values.
 */
void Bot::_resetInfos()
{
	_target.clear();
	_clientNickname.clear();
	_channelName.clear();

	_input.clear();
	_command.clear();
	_ageArg.clear();
	_commandPos = 0;
	
	_year = 0;
	_month = 0;
	_day = 0;
	_currentYear = 0;
	_currentMonth = 0;
	_currentDay = 0;
}