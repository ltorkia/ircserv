/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:38:18 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

// === COMMAND NAMES ===
namespace commands
{
	const std::string PASS 					= "PASS";
	const std::string NICK 					= "NICK";
	const std::string USER 					= "USER";
	const std::string CAP					= "CAP";
	const std::string INVITE 				= "INVITE";
	const std::string JOIN 					= "JOIN";
	const std::string TOPIC 				= "TOPIC";
	const std::string KICK					= "KICK";
	const std::string PART 					= "PART";
	const std::string MODE					= "MODE";
	const std::string PRIVMSG 				= "PRIVMSG";
	const std::string PING 					= "PING";
	const std::string PONG 					= "PONG";
	const std::string WHOIS 				= "WHOIS";
	const std::string WHOWAS 				= "WHOWAS";
	const std::string WHO 					= "WHO";
	const std::string AWAY 					= "AWAY";
	const std::string QUIT		 			= "QUIT";
	const std::string DCC					= "DCC";
}