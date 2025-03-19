NAME				= 	ircserv

#-----> MANDATORY

SRCS_DIR			=	srcs
OBJS_DIR			= 	build

CORE_DIR			=	core
CMD_DIR				=	commands
UTILS_DIR			=	utils

CORE_FILES  		=	Server.cpp		Client.cpp		Channel.cpp

CMD_FILES			=	CommandHandler.cpp				CommandHandler_Auth.cpp \
						CommandHandler_Channel.cpp 		CommandHandler_File.cpp \
						CommandHandler_Log.cpp 			CommandHandler_Message.cpp \
						CommandHandler_Mode.cpp

UTILS_FILES			=	MessageHandler.cpp		IrcHelper.cpp		Utils.cpp

MAIN_FILES			=	main.cpp \
						$(addprefix $(CORE_DIR)/, $(CORE_FILES)) \
						$(addprefix $(CMD_DIR)/, $(CMD_FILES)) \
						$(addprefix $(UTILS_DIR)/, $(UTILS_FILES))

#########################################################
#############         COMPILATION            ############
#########################################################

SRCS				= 	${addprefix $(SRCS_DIR)/,$(MAIN_FILES)}
OBJS				= 	${SRCS:%.cpp=${OBJS_DIR}/%.o}
DEPS				= 	${OBJS:.o=.d}
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

#-----> MANDATORY

${OBJS_DIR}/%.o: %.cpp
	@mkdir -p ${dir $@}
	@echo "\n${GREEN}--> Compiling $<${RESET}"
	${CXX} -MMD -c ${CXXFLAGS} $< -o $@

all: ${NAME}

${NAME}: ${OBJS}
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                    LINKING                    ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	@echo "${GREEN}--> ${NAME}${RESET}\n"
	${CXX} ${OBJS} -o ${NAME}

#-----> BONUS

$(OBJS_DIRB)/%.o: %.cpp
	@mkdir -p ${dir $@}
	@echo "\n${GREEN}--> Compiling Bonus $<${RESET}"
	${CXX} -MMD -c ${CXXFLAGS} $< -o $@

bonus: ${NAME_BONUS}

${NAME_BONUS}: ${OBJSB}
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                    LINKING                    ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	@echo "${GREEN}--> ${NAME_BONUS}${RESET}\n"
	${CXX} ${OBJSB} -o ${NAME_BONUS}

#-----> CLEANING / RECOMPILATION

clean:
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                  CLEANING                     ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	${RM} ${OBJS_DIR}
	${RM} ${OBJS_DIRB}

fclean:
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                 FULL CLEAN                    ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	${RM} ${OBJS_DIR}
	${RM} ${NAME}
	${RM} ${OBJS_DIRB}
	${RM} ${NAME_BONUS}

re: fclean
	@$(MAKE) all

#-----> DEBUG

debug: fclean
	@$(MAKE) CXXFLAGS="$(CXXFLAGS_DEBUG)" all

#-----> INCLUDE DEPENDENCIES

-include ${DEPS}

.PHONY: all clean fclean re debug