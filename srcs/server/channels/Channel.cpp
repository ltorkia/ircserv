/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:33:07 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

// =========================================================================================

// === CONSTUCTOR / DESTRUCTOR ===

// ========================================= PUBLIC ========================================

Channel::Channel(const std::string &name, const std::string& password) :
	_name(name),
	_password(password),
	_topic(""),
	_channelTimestamp(time(0)),
	_isInviteOnly(false),
	_isSettableTopic(false),
	_clientsLimit(-1) {}

Channel::~Channel() {}

// ========================================= PRIVATE =======================================

Channel::Channel() {}
Channel::Channel(const Channel& src) {(void) src;}
Channel & Channel::operator=(const Channel& src) {(void) src; return *this;}