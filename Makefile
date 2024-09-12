# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pviegas <pviegas@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/08/21 13:22:14 by pviegas           #+#    #+#              #
#    Updated: 2024/09/12 12:22:50 by pviegas          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS		=	./srcs/ircserv.cpp ./srcs/Server.cpp ./srcs/Client.cpp ./srcs/Utils.cpp ./srcs/Channel.cpp

OBJS		= $(SRCS:.cpp=.o)

NAME		=	ircserv

CPP			=	c++

CPPFLAGS		=	-Wall -Wextra -Werror -std=c++98 -g #-fsanitize=address

RM			=	rm -rf

INC			=	-Iincludes -I/usr/include

# /* ~~~~~~~ Colors ~~~~~~~ */
BLACK:="\033[1;30m"
RED:="\033[1;31m"
GREEN:="\033[1;32m"
PURPLE:="\033[1;35m"
CYAN:="\033[1;36m"
WHITE:="\033[1;37m"
EOC:="\033[0;0m"


all: $(NAME)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) $(INC) -c $< -o $@

$(NAME): $(OBJS)
	$(CPP) $(OBJS) $(CPPFLAGS) $(INC) -o $(NAME)
	@echo $(GREEN) "[Ex compiled!]" $(EOC)

clean:
	@echo $(PURPLE) "[🧹Cleaning...🧹]" $(EOC)
	$(RM) $(OBJS)

fclean: clean
	@echo $(PURPLE) "[🧹FCleaning...🧹]" $(EOC)
	$(RM) $(NAME)
	$(RM) *.out
	@echo $(GREEN) "[All cleaned!]" $(EOC)

re: fclean all

# Run executable
run: all
	@clear
	./$(NAME) 8090 123

# valgrind
val: fclean all
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) 8090 123

.PHONY: all clean fclean re run val
