NAME_SERVER	= ircserv
NAME_BOT	= ircbot

SRCS_DIR			=	srcs
OBJS_DIR			= 	build
OBJS_DIR_BOT		= 	build_bot

CORE_DIR			=	core
CMD_DIR				=	commands
BOT_DIR				=	bot
UTILS_DIR			=	utils

CORE_FILES  		=	Server.cpp		Client.cpp		Channel.cpp

CMD_FILES			=	CommandHandler.cpp				CommandHandler_Auth.cpp \
						CommandHandler_Channel.cpp 		CommandHandler_File.cpp \
						CommandHandler_Log.cpp 			CommandHandler_Message.cpp \
						CommandHandler_Mode.cpp

BOT_FILES  			=	main.cpp				Bot.cpp							Bot_MessageStream.cpp \
						Bot_Authenticate.cpp	Bot_CommandHandlerServer.cpp	Bot_CommandHandlerUser \
						Bot_ParsingHelper.cpp

UTILS_FILES			=	MessageHandler.cpp		IrcHelper.cpp		Utils.cpp

MAIN_SERVER_FILES	=	main.cpp \
						$(addprefix $(CORE_DIR)/, $(CORE_FILES)) \
						$(addprefix $(CMD_DIR)/, $(CMD_FILES)) \
						$(addprefix $(UTILS_DIR)/, $(UTILS_FILES))

MAIN_BOT_FILES		=	$(addprefix $(BOT_DIR)/, $(BOT_FILES)) \
						$(addprefix $(CORE_DIR)/, $(CORE_FILES)) \
						$(addprefix $(CMD_DIR)/, $(CMD_FILES)) \
						$(addprefix $(UTILS_DIR)/, $(UTILS_FILES))

#########################################################
#############         COMPILATION            ############
#########################################################

#-----> SERVEUR IRC

SRCS				= 	${addprefix $(SRCS_DIR)/,$(MAIN_SERVER_FILES)}
OBJS				= 	${SRCS:%.cpp=${OBJS_DIR}/%.o}
DEPS				= 	${OBJS:.o=.d}

#-----> BOT

SRCS_BOT			= 	${addprefix $(SRCS_DIR)/,$(MAIN_BOT_FILES)}
OBJS_BOT			= 	${SRCS_BOT:%.cpp=${OBJS_DIR_BOT}/%.o}
DEPS_BOT			= 	${OBJS_BOT:.o=.d}

INC_DIRS			= 	-I./incs/classes/ -I./incs/config/

CXX					= 	c++
CXXFLAGS			= 	-Wall -Wextra -Werror -std=c++98 $(INC_DIRS)
CXXFLAGS_DEBUG		=	$(CXXFLAGS) -g3 -DDEBUG

RM					= 	rm -rf


#########################################################
#############            COLORS               ###########
#########################################################

GREEN				= \033[1;32m
CYAN				= \033[1;36m
RESET				= \033[0m


#########################################################
#############             RULES               ###########
#########################################################

#-----> SERVEUR IRC

${OBJS_DIR}/%.o: %.cpp
	@mkdir -p ${dir $@}
	@echo "\n${GREEN}--> Compiling $<${RESET}"
	${CXX} -MMD -c ${CXXFLAGS} $< -o $@

all: ${NAME_SERVER}
	@$(MAKE) bot

${NAME_SERVER}: ${OBJS}
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                    LINKING                    ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	@echo "${GREEN}--> ${NAME_SERVER}${RESET}\n"
	${CXX} ${OBJS} -o ${NAME_SERVER}

#-----> BOT

# Règle pour compiler les fichiers du bot
$(OBJS_DIR_BOT)/%.o: %.cpp
	@mkdir -p ${dir $@}
	@echo "\n${GREEN}--> Compiling ircbot $<${RESET}"
	${CXX} -MMD -c ${CXXFLAGS} $< -o $@

bot: ${NAME_BOT}

${NAME_BOT}: ${OBJS_BOT}
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                    LINKING                    ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	@echo "${GREEN}--> ${NAME_BOT}${RESET}\n"
	${CXX} ${OBJS_BOT} -o ${NAME_BOT}

#-----> CLEANING / RECOMPILATION

clean:
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                  CLEANING                     ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	${RM} ${OBJS_DIR}
	${RM} ${OBJS_DIR_BOT}

fclean:
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                 FULL CLEAN                    ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	${RM} ${OBJS_DIR}
	${RM} ${NAME_SERVER}
	${RM} ${OBJS_DIR_BOT}
	${RM} ${NAME_BOT}

re: fclean
	@$(MAKE) all

#-----> DEBUG

debug: fclean
	@$(MAKE) CXXFLAGS="$(CXXFLAGS_DEBUG)" all

#-----> INCLUDE DEPENDENCIES

-include ${DEPS}

.PHONY: all clean fclean re debug bot