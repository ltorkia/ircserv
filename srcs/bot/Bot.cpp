/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:32:47 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"

// === OTHER CLASSES ===
#include "Utils.hpp"

// === NAMESPACES ===
#include "irc_config.hpp"
#include "bot_config.hpp"
#include "server_messages.hpp"
#include "colors.hpp"

using namespace bot_config;
using namespace server_messages;
using namespace colors;

// =========================================================================================

// === BOT SETTINGS ===

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


// === CONSTRUCTOR / DESTRUCTOR ===

/**
 * @brief Constructs a Bot object and initializes its attributes.
 * 
 * This constructor sets up the bot's file descriptor, nickname, username, 
 * real name, and mask. It also initializes various flags related to the 
 * bot's authentication state. Additionally, it sets up signal handling.
 * 
 * @param botFd The file descriptor associated with the bot's connection.
 * @param nick The nickname of the bot.
 * @param user The username of the bot.
 * @param real The real name of the bot.
 */
Bot::Bot(int botFd, const std::string& nick, const std::string& user, const std::string& real)
	: _botFd(botFd), _serverMsgCount(0), _isAuthenticated(false),
	_botNick(nick), _botUser(user), _botReal(real),
	_botMask(_botNick + "!" + '~' + _botUser + "@" + Utils::getEnvValue(env::SERVER_IP_KEY))
{
	_setSignal();
}
Bot::~Bot()
{
	if (close(_botFd) == -1)
		perror("Failed to close bot socket");
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
 * - Sends authentication information to the server.
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
	_sendAuthInfos();

	while (true)
	{
		_resetAttributes();
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


// ========================================= PRIVATE =======================================

Bot::Bot() {}
Bot::Bot(const Bot& src) {(void) src;}
Bot & Bot::operator=(const Bot& src) {(void) src; return *this;}


// === BASIC SETTINGS ===

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
void Bot::_setSignal()
{
	struct sigaction sa;
	sa.sa_handler = &Bot::signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTSTP, &sa, NULL) == -1)
		throw std::runtime_error(ERR_SET_SIGNAL);
}

/**
 * @brief Resets the internal state of the Bot object.
 *
 * This function clears all the internal data members of the Bot object,
 * including target, client nickname, channel name, input, command, age argument,
 * and resets the command position and date-related members to their initial values.
 */
void Bot::_resetAttributes()
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