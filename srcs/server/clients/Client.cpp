#include "../../../incs/server/Client.hpp"

// =========================================================================================

// === CONSTUCTOR / DESTRUCTOR ==

// ========================================= PUBLIC ========================================

Client::Client(int fd) :
	_clientSocketFd(fd),
	_isIrssi(false),
	_isIdentified(false),
	_authenticated(false),
	_rightPassServ(false),
	_signonTime(time(NULL)),
	_lastActivity(time(NULL)),
	_isAway(false),
	_errorMsgTooLongSent(false),
	_pingSent(false) {}

Client::~Client() {}

// ========================================= PRIVATE =======================================

Client::Client() {}
Client::Client(const Client& src) {(void) src;}
Client & Client::operator=(const Client& src) {(void) src; return *this;}