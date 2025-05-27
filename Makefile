NAME = ircserv

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp Server.cpp Client.cpp Channel.cpp ServerCommands.cpp ChannelCommands.cpp \
		modes.cpp

OBJS = $(SRCS:.cpp=.o)

DEP = ${SRCS:.cpp=.d}

RM = rm -rf

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

# -MMD includes only user headers in .d files
# -MD includes also system headers
# -MP when header gets deleted, it does not stop the make

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MP

# '-' in -include means that it's gonna ignore errors
# for running make first time, when .d files are not genareted yet
-include $(DEP)

clean :
	$(RM) $(OBJS) $(DEP)

fclean : clean
	$(RM) $(NAME) $(DEP)

re : fclean all

.PHONY : all clean fclean re
