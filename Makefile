NAME = ircserv

CC = c++

FLAGS = -g -Wall -Wextra -Werror -std=c++98

INCLUDES = -I ./Includes/

SERVER_SOURCES = ./Server/Server.cpp \
				 ./Server/User.cpp \
				 ./Server/main.cpp \
				 ./Server/Channel.cpp \

COMMANDES_SOURCES = ./Commandes/Commandes.cpp \
					./Commandes/CommandesUtils.cpp \


SERVER_OBJ_PATH = obj/Server/
COMMANDES_OBJ_PATH = obj/Commandes/

SERVER_OBJS = ${SERVER_SOURCES:.cpp=.o}
SERVER_OBJS := $(addprefix ${SERVER_OBJ_PATH}, $(notdir ${SERVER_OBJS}))

COMMANDES_OBJS = ${COMMANDES_SOURCES:.cpp=.o}
COMMANDES_OBJS := $(addprefix ${COMMANDES_OBJ_PATH}, $(notdir ${COMMANDES_OBJS}))

all : ${SERVER_OBJ_PATH} ${COMMANDES_OBJ_PATH} ${NAME}


${SERVER_OBJ_PATH}:
	mkdir -p ${SERVER_OBJ_PATH}

${COMMANDES_OBJ_PATH}:
	mkdir -p ${COMMANDES_OBJ_PATH}

${NAME} : ${SERVER_OBJS} ${COMMANDES_OBJS}
	${CC} ${FLAGS} ${SERVER_OBJS} ${COMMANDES_OBJS}	-o $@

${SERVER_OBJ_PATH}%.o : ./Server/%.cpp
	${CC} ${FLAGS} ${INCLUDES} -c $< -o $@

${COMMANDES_OBJ_PATH}%.o : ./Commandes/%.cpp
	${CC} ${FLAGS} ${INCLUDES} -c $< -o $@

clean : 
	${RM} ${SERVER_OBJS} ${COMMANDES_OBJS}
	${RM} -r obj/

fclean : clean
	${RM} ${NAME}

re : fclean all

.PHONY : all clean fclean re
