NAME = ircserv

CC = c++

FLAGS = -g -Wall -Wextra -Werror -std=c++98

INCLUDES = -I ./Includes/

PARSING_SOURCES = ./Parsing/
SERVER_SOURCES = ./Server/
COMMANDES_SOURCES = ./Commandes/


PARSING_OBJ_PATH = obj/Parsing/
SERVER_OBJ_PATH = obj/Server/
COMMANDES_OBJ_PATH = obj/Commandes/

PARSING_OBJS = ${PARSING_SOURCES:.cpp=.o}
PARSING_OBJS := $(addprefix ${PARSING_OBJ_PATH}, $(notdir ${PARSING_OBJS}))

SERVER_OBJS = ${SERVER_SOURCES:.cpp=.o}
SERVER_OBJS := $(addprefix ${SERVER_OBJ_PATH}, $(notdir ${SERVER_OBJS}))

COMMANDES_OBJS = ${COMMANDES_SOURCES:.cpp=.o}
COMMANDES_OBJS := $(addprefix ${COMMANDES_OBJ_PATH}, $(notdir ${COMMANDES_OBJS}))

all : ${PARSING_OBJ_PATH} ${SERVER_OBJ_PATH} ${COMMANDES_OBJ_PATH} ${NAME}

${PARSING_OBJ_PATH}:
	mkdir -p ${PARSING_OBJ_PATH}

${SERVER_OBJ_PATH}:
	mkdir -p ${SERVER_OBJ_PATH}

${COMMANDES_OBJ_PATH}:
	mkdir -p ${COMMANDES_OBJ_PATH}

${NAME} : ${PARSING_OBJS} ${SERVER_OBJS} ${COMMANDES_OBJS}
	${CC} ${FLAGS} ${PARSING_OBJS} ${SERVER_OBJS} ${COMMANDES_OBJS} \
	-o $@ ${MLX_FLAG} ${LIBFT_FLAG}

${PARSING_OBJ_PATH}%.o : ./Parsing/%.cpp
	${CC} ${FLAGS} ${INCLUDES} -c $< -o $@

${SERVER_OBJ_PATH}%.o : ./Server/%.cpp
	${CC} ${FLAGS} ${INCLUDES} -c $< -o $@

${COMMANDES_OBJ_PATH}%.o : ./Commandes/%.cpp
	${CC} ${FLAGS} ${INCLUDES} -c $< -o $@

clean : 
	${RM} ${PARSING_OBJS} ${SERVER_OBJS} ${COMMANDES_OBJS}
	${RM} -r obj/

fclean : clean
	${RM} ${NAME}

re : fclean all

.PHONY : all clean fclean re
