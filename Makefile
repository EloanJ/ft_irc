# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vduarte <vduarte@student.42mulhouse.fr>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/03 12:44:32 by vduarte           #+#    #+#              #
#    Updated: 2026/01/14 13:41:30 by vduarte          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g3

SRC = safeSend.cpp Server.cpp Bot.cpp Channel.cpp Client.cpp main.cpp

OBJ = $(SRC:%.cpp=obj/%.o)

# Colors
GREEN = \033[1;32m
BLUE = \033[1;34m
RED = \033[1;31m
YELLOW = \033[1;33m
NC = \033[0m

all : ${NAME}

${NAME} : ${OBJ}
		@${CXX} -o ${NAME} ${OBJ} ${CXXFLAGS}
		@echo "${GREEN}💬 Compilation de ${NAME} terminée 💬${NC}"

obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	@${CXX} ${CXXFLAGS} -c $< -o $@
	@echo "${BLUE}🔨 Compilation de $< réussie 🔨${NC}"

clean :
		@rm -rf obj
		@echo "${YELLOW}🧹 Objet(s) supprimé(s) 🧹${NC}"

fclean : clean
		@rm -rf ${NAME}
		@echo "${RED}🗑️  Exécutable supprimé 🗑️${NC}"

re : fclean all

.PHONY : all clean fclean re