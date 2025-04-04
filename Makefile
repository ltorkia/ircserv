#-----> PROGRAM NAMES (SERVER + BOT)
NAME_SERVER			=	ircserv
NAME_BOT			=	ircbot
NAME_LIB			=	libtools.a

#########################################################

#-----> DIRECTORY NAMES (SRCS, OBJS, OBJS_BOT, COMMON_OBJS)
SRCS_DIR			=	srcs
OBJS_DIR			=	bin

#-----> COMMON LIBRARY (SHARED BY SERVER + BOT)
LIB_TOOLS			=	$(OBJS_DIR)/$(NAME_LIB)

#########################################################

#-----> SOURCES DIRECTORIES
#---> FIRST LEVEL
SERVER_DIR			=	server
BOT_DIR				=	bot
UTILS_DIR			=	utils
#---> SECOND LEVEL
CORE_DIR			=	core
CHANNELS_DIR		=	channels
CLIENTS_DIR			=	clients
CMD_DIR				=	commands

#########################################################

#-----> ALL SOURCES FILES
SERVER_MAIN			=	ServerMain.cpp

SERVER_FILES		=	Server.cpp						Server_Clients.cpp \
						Server_Infos.cpp				Server_Loop.cpp

CHANNELS_FILES		=	Channel.cpp						Channel_Attributes.cpp \
						Channel_Actions.cpp

CLIENTS_FILES		=	Client.cpp						Client_Attributes.cpp \
						Client_Actions.cpp

COMMON_UTILS_FILES	=	MessageBuilder.cpp				Utils.cpp

SERVER_UTILS_FILES	=	IrcHelper.cpp

CMD_FILES			=	Command.cpp						Command_Register.cpp \
						Command_Channel.cpp				Command_File.cpp \
						Command_Mode.cpp 				Command_Message.cpp \
						Command_Log.cpp

BOT_FILES			=	BotMain.cpp						Bot.cpp \
						Bot_Message.cpp					Bot_Register.cpp \
						Bot_Parser.cpp					Bot_Command.cpp

#########################################################

#-----> JOIN SOURCES FOR EACH PROGRAM OR COMMON USE
COMMON_FILES		=	$(addprefix $(UTILS_DIR)/, $(COMMON_UTILS_FILES))

MAIN_SERVER_FILES	=	$(addprefix $(SERVER_DIR)/, $(SERVER_MAIN)) \
						$(addprefix $(SERVER_DIR)/, $(addprefix $(CORE_DIR)/, $(SERVER_FILES))) \
						$(addprefix $(SERVER_DIR)/, $(addprefix $(CHANNELS_DIR)/, $(CHANNELS_FILES))) \
						$(addprefix $(SERVER_DIR)/, $(addprefix $(CLIENTS_DIR)/, $(CLIENTS_FILES))) \
						$(addprefix $(SERVER_DIR)/, $(addprefix $(CMD_DIR)/, $(CMD_FILES))) \
						$(addprefix $(UTILS_DIR)/, $(SERVER_UTILS_FILES))

MAIN_BOT_FILES		=	$(addprefix $(BOT_DIR)/, $(BOT_FILES))


#########################################################
#############         COMPILATION            ############
#########################################################

#-----> COMMON LIB
COMMON_SRCS			=	$(COMMON_FILES)
COMMON_OBJS			=	${COMMON_SRCS:%.cpp=${OBJS_DIR}/%.o}
COMMON_DEPS			=	${COMMON_OBJS:.o=.d}

#-----> SERVEUR IRC
SRCS				=	$(MAIN_SERVER_FILES)
OBJS				=	${SRCS:%.cpp=${OBJS_DIR}/%.o}
DEPS				=	${OBJS:.o=.d}

#-----> BOT
SRCS_BOT			=	$(MAIN_BOT_FILES)
OBJS_BOT			=	${SRCS_BOT:%.cpp=${OBJS_DIR}/%.o}
DEPS_BOT			=	${OBJS_BOT:.o=.d}

#########################################################

#-----> INCLUDES
INC_DIR				=	incs
CONFIG_DIR			=	config
INC_DIRS			=	-I./$(INC_DIR)/$(SERVER_DIR) \
						-I./$(INC_DIR)/$(CMD_DIR) \
						-I./$(INC_DIR)/$(BOT_DIR) \
						-I./$(INC_DIR)/$(UTILS_DIR) \
						-I./$(INC_DIR)/$(CONFIG_DIR)

#########################################################

#-----> COMPILATION FLAGS
CXX					=	c++
CXXFLAGS			=	-Wall -Wextra -Werror -std=c++98 $(INC_DIRS)
CXXFLAGS_DEBUG		=	$(CXXFLAGS) -g3 -DDEBUG

#-----> CLEANING
RM					=	rm -rf


#########################################################
#############            COLORS               ###########
#########################################################

GREEN				=	\033[1;32m
CYAN				=	\033[1;36m
RESET				=	\033[0m


#########################################################
#############             RULES               ###########
#########################################################

#-----> MAKE ALL RULE
all: $(LIB_TOOLS) server bot

#-----> SERVEUR IRC
${OBJS_DIR}/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p ${dir $@}
	@echo "\n${GREEN}--> Compiling server $<${RESET}"
	${CXX} -MMD -c ${CXXFLAGS} $< -o $@

server: $(NAME_SERVER)

${NAME_SERVER}: ${OBJS} $(LIB_TOOLS)
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                LINKING SERVER                 ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	@echo "${GREEN}--> ${NAME_SERVER}${RESET}\n"
	${CXX} ${OBJS} -o ${NAME_SERVER} -L$(OBJS_DIR) -ltools

#########################################################

#-----> BOT
${OBJS_DIR}/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p ${dir $@}
	@echo "\n${GREEN}--> Compiling ircbot $<${RESET}"
	${CXX} -MMD -c ${CXXFLAGS} $< -o $@

bot: ${NAME_BOT}

${NAME_BOT}: ${OBJS_BOT} $(LIB_TOOLS)
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                  LINKING BOT                  ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	@echo "${GREEN}--> ${NAME_BOT}${RESET}\n"
	${CXX} ${OBJS_BOT} -o ${NAME_BOT} -L$(OBJS_DIR) -ltools

#########################################################

#-----> COMMON OBJECTS
${OBJS_DIR}/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p ${dir $@}
	@echo "\n${GREEN}--> Compiling library $<${RESET}"
	${CXX} -MMD -c ${CXXFLAGS} $< -o $@

#-----> COMMON LIBRARY (libcommon.a)
$(LIB_TOOLS): $(COMMON_OBJS)
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####               CREATING LIBRARY                ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	@echo "${GREEN}--> $(NAME_LIB)${RESET}\n"
	@mkdir -p $(dir $(LIB_TOOLS))
	ar rcs $(LIB_TOOLS) $(COMMON_OBJS) 

#########################################################

#-----> CLEANING / RECOMPILATION
clean:
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                  CLEANING                     ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	${RM} ${DEPS} ${DEPS_BOT} ${COMMON_DEPS}
	${RM} -rf ${OBJS_DIR}

fclean: clean
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                 FULL CLEAN                    ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	${RM} ${NAME_SERVER} ${NAME_BOT} $(LIB_TOOLS)

re: fclean
	@echo "\n"
	@echo "${CYAN}#######################################################${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}####                  REBUILD                      ####${RESET}"
	@echo "${CYAN}####                                               ####${RESET}"
	@echo "${CYAN}#######################################################${RESET}\n"
	$(MAKE) all

#-----> DEBUG
debug: fclean
	@$(MAKE) CXXFLAGS="$(CXXFLAGS_DEBUG)" all

#########################################################

#-----> INCLUDE DEPENDENCIES
-include ${DEPS} ${DEPS_BOT} ${COMMON_DEPS}

.PHONY: all clean fclean re debug server bot