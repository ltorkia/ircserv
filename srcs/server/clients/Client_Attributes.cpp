/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client_Attributes.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:33:14 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

// === OTHER CLASSES ===
#include "Channel.hpp"

// =========================================================================================

// === CLIENT INFOS

// =========================================================================================

// === CLIENT INFOS SETTERS ===

// === GENERAL INFOS ===

void Client::setClientPort(int port)
{
	_port = port;
}
void Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}
void Client::setUsername(const std::string &username)
{
	_username = username;
}
void Client::setRealName(const std::string &realName)
{
	_realName = realName;
}
void Client::setHostname(const std::string &hostname)
{
	_hostname = hostname;
}
void Client::setClientIp(const std::string &clientIp)
{
	_clientIp = clientIp;
}
void Client::setUsermask()
{
	_usermask = _nickname + "!" + _username + "@" + _clientIp;
}


// === AUTHENTICATION INFOS ===

void Client::setIsIrssi(bool status)
{
	_isIrssi = status;
}
void Client::setIdentified(bool status)
{
	_isIdentified = status;
}
void Client::setIdentNickCmd(std::vector<std::string> identCmd)
{
	_identNicknameCmd = identCmd;
}
void Client::setIdentUsernameCmd(std::vector<std::string> identCmd)
{
	_identUsernameCmd = identCmd;
}
void Client::setServPasswordValidity(bool status)
{
	_rightPassServ = status;
}
void Client::authenticate()
{
	_authenticated = true;
}


// === ACTIVITY INFOS ===

void Client::setLastActivity()
{
	_lastActivity = time(NULL);
}
void Client::setIsAway(bool status)
{
	_isAway = status;
}
void Client::setAwayMessage(const std::string& message)
{
	_awayMessage = message;
}
void Client::setErrorMsgTooLongSent(bool status)
{
	_errorMsgTooLongSent = status;
}
void Client::setPingSent(bool status)
{
	_pingSent = status;
}

// =========================================================================================

// === CLIENT INFOS GETTERS ===

// === GENERAL INFOS ===

int Client::getFd() const
{
	return _clientSocketFd;
}
int Client::getClientPort() const
{
	return _port;
}
const std::string& Client::getNickname() const
{
	return _nickname;
}
const std::string& Client::getUsername() const
{
	return _username;
}
const std::string& Client::getRealName() const
{
	return _realName;
}
const std::string& Client::getHostname() const
{
	return _hostname;
}
const std::string& Client::getClientIp() const
{
	return _clientIp;
}
const std::string& Client::getUsermask() const
{
	return _usermask;
}


// === AUTHENTICATION INFOS ===

bool Client::isIrssi() const
{
	return _isIrssi;
}
bool Client::isIdentified() const
{
	return _isIdentified;
}
std::vector<std::string> Client::getIdentNickCmd() const
{
	return _identNicknameCmd;
}
std::vector<std::string> Client::getIdentUsernameCmd() const
{
	return _identUsernameCmd;
}
bool Client::gotValidServPassword() const
{
	return _rightPassServ;
}
bool Client::isAuthenticated() const
{
	return _authenticated;
}


// === ACTIVITY INFOS ===

time_t Client::getSignonTime() const
{
	return _signonTime;
}
time_t Client::getLastActivity() const
{
	return _lastActivity;
}
time_t Client::getIdleTime() const
{
	return time(NULL) - _lastActivity;
}
bool Client::isAway() const
{
	return _isAway;
}
const std::string& Client::getAwayMessage() const
{
	return _awayMessage;
}
bool Client::errorMsgTooLongSent() const
{
	return _errorMsgTooLongSent;
}
bool Client::pingSent() const
{
	return _pingSent;
}


// === RELATED CHANNELS ===

std::map<std::string, Channel*>& Client::getChannelsJoined()
{
	return _channelsJoined;
}
bool Client::isInChannel(const std::string& channelName) const
{
	return _channelsJoined.find(channelName) != _channelsJoined.end();
}
bool Client::isOperator(Channel* channel) const
{
	return channel->isOperator(this);
}
bool Client::isInvited(const Channel* channel) const
{
	return channel->isInvited(this);
}


// === BUFFER ===

std::string& Client::getBufferMessage()
{
	return _bufferMessage;
}