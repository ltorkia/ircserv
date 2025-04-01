/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   colors.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltorkia <ltorkia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 10:44:25 by ltorkia           #+#    #+#             */
/*   Updated: 2025/04/01 08:38:21 by ltorkia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

namespace colors
{
	// =================================================================================
	
	// === IRC COLOR CODES ===

	// =================================================================================

	const std::string IRC_RESET 			= "\x0F";
	const std::string IRC_RED 				= "\x03""04";
	const std::string IRC_GREEN 			= "\x03""03";
	const std::string IRC_YELLOW 			= "\x03""08";
	const std::string IRC_BLUE 				= "\x03""02";
	const std::string IRC_DARK_BLUE 		= "\x03""12";
	const std::string IRC_PURPLE 			= "\x03""06";
	const std::string IRC_CYAN 				= "\x03""10";
	const std::string IRC_WHITE 			= "\x03""00";

	const std::string IRC_DEFAULT 			= IRC_WHITE;
	const std::string IRC_COLOR_SUCCESS 	= IRC_GREEN;
	const std::string IRC_COLOR_ERR 		= IRC_RED;
	const std::string IRC_COLOR_PROMPT 		= IRC_CYAN;
	const std::string IRC_COLOR_INFO 		= IRC_BLUE;
	const std::string IRC_COLOR_DISPLAY 	= IRC_YELLOW;
	const std::string IRC_COLOR_LINE 		= IRC_DARK_BLUE;


	// =================================================================================
	
	// === SERVER UNIX COLOR CODES ===

	// =================================================================================

	const std::string RESET 				= "\033[0m";
	const std::string RED 					= "\033[31m";
	const std::string GREEN 				= "\033[32m";
	const std::string YELLOW 				= "\033[33m";
	const std::string BLUE 					= "\033[34m";
	const std::string DARK_BLUE 			= "\033[94m";
	const std::string PURPLE 				= "\033[95m";
	const std::string CYAN 					= "\033[36m";
	const std::string WHITE 				= "\033[37m";

	const std::string DEFAULT 				= WHITE;
	const std::string COLOR_SUCCESS 		= GREEN;
	const std::string COLOR_ERR 			= RED;
	const std::string COLOR_PROMPT 			= CYAN;
	const std::string COLOR_INFO 			= BLUE;
	const std::string COLOR_DISPLAY 		= YELLOW;
	const std::string COLOR_LINE 			= DARK_BLUE;
}