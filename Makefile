# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/08/21 13:22:14 by pviegas           #+#    #+#              #
#    Updated: 2024/09/25 10:33:17 by pviegas          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Color variables
BLACK   = \033[1;30m
RED		= \033[1;31m
GREEN	= \033[1;32m
YELLOW	= \033[1;33m
BLUE	= \033[1;34m
PURPLE  = \033[1;35m
CYAN    = \033[1;36m
WHITE	= \033[1;37m
RESET 	= \033[0m

# Executable name
NAME = ircserv

# Compiler options
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -g3 #-fsanitize=address
INCLUDES	= -I ./include

# Paths
SRC_DIR = srcs/
OBJ_DIR = objs/

# Files
SRC_FILES =	ircserv.cpp \
			Server.cpp \
			Client.cpp \
			Channel.cpp \
			Utils.cpp

OBJS = $(addprefix $(OBJ_DIR), $(SRC_FILES:.cpp=.o))

# Compile object files
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@clear
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDES)

# Default target
all: $(NAME)

# Link executable
${NAME}: $(OBJS)
	@echo "\n$(BLUE) [Compiling] $(NAME)$(RESET)\n"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN) [Success] $(NAME) created.$(RESET)\n\n"

# Clean object files
clean:
	@clear
	@rm -rf $(OBJS)
	@echo "\n$(RED) [Deleting] .o files ... (deleted)$(RESET)\n\n"

# Clean all files
fclean: clean
	@rm -rf $(NAME)
	@echo "$(GREEN) [Success] $(NAME) files ... (deleted)$(RESET)\n\n"

# Rebuild
re: fclean all

# Run executable
run: all
	@clear
	./$(NAME) 8090 123

# valgrind
val: fclean all
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes ./$(NAME) 8090 123

.PHONY: all clean fclean re run val