/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:33:18 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

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