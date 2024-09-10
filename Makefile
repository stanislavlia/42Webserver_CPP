NAME		:= webserver

SRCDIR		:= src
OBJDIR		:= object
HDRDIR		:= include
HDR			:= $(wildcard $(HDRDIR)/*.hpp)
INC			:= -I./include

CXX			:= c++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++98

SRCS		:= $(wildcard $(SRCDIR)/*.cpp)
OBJS		:= $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS		:= $(patsubst %.cpp,$(OBJDIR)/%.d,$(SRCS))

MKDIR		:= mkdir -p
RM			:= rm -rf

# Define color codes for output messages
YELLOW		:= "\033[1;33m"
GREEN		:= "\033[1;32m"
END			:= "\033[0m"

.PHONY: all clean fclean re

all: $(NAME)

# Main target
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo $(GREEN) "\n\tProject is compiled\n" ${END}

$(OBJDIR)/%.o: ./%.cpp $(HDR)
	$(MKDIR) $(@D)
	$(CXX) $(CXXFLAGS) $(INC) -g -c -o $@ $< -MD

clean:
	$(RM) $(OBJDIR)
	@echo $(YELLOW) "\n\tProject 'clean' status: DONE\n" ${END}

fclean: clean
	$(RM) $(NAME)
	@echo $(YELLOW) "\n\tProject 'clean' status: DONE\n" ${END}

re: fclean all