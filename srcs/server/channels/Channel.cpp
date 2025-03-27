#include "../../../incs/server/Channel.hpp"

// =========================================================================================

// === CONSTUCTOR / DESTRUCTOR ===

// ========================================= PUBLIC ========================================


Channel::Channel(const std::string &name, const std::string& password) :
	_name(name),
	_password(password),
	_topic(""),
	_channelTimestamp(time(0)),
	_invites(false),
	_rightsTopic(false),
	_limits(-1) {}

Channel::~Channel() {}

// ========================================= PRIVATE =======================================

Channel::Channel() {}
Channel::Channel(const Channel& src) {(void) src;}
Channel & Channel::operator=(const Channel& src) {(void) src; return *this;}