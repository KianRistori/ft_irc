CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98

SRCS = src/User.cpp src/Channel.cpp src/handleIrcCommand.cpp src/modeSet.cpp src/main.cpp

OBJS = ${SRCS:.cpp=.o}

MAIN = ircserv

all: ${MAIN}
		@echo   ${MAIN} has been compiled

${MAIN}: ${OBJS}
		${CXX} ${CXXFLAGS} ${OBJS} -o ${MAIN}

.cpp.o:
		${CXX} ${CXXFLAGS} -c $< -o $@

clean:
		${RM} ${OBJS} *.o *~.

fclean: clean
		${RM} ${MAIN}

re:		fclean all
