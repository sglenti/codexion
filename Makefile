# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/20 19:26:08 by slenti            #+#    #+#              #
#    Updated: 2026/08/13 10:34:49 by slenti           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# -------------------------------------------------
#  Variables
# -------------------------------------------------
CC = cc
CFLAGS = -g -Wall -Wextra -Werror -pthread -I$(INC_DIR)
RM = rm -f

NAME = codexion
SRCS_DIR := srcs
OBJS_DIR := build
INC_DIR := includes

SRCS := main.c error.c validate.c simulation.c monitor.c coders.c dongles.c \
		logger.c coder_routine.c adquire.c heap.c

SRCS_FULL := $(addprefix $(SRCS_DIR)/,$(SRCS))
OBJS := $(patsubst $(SRCS_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRCS_FULL))

#LIB_DIR = libft
#LIB_NAME = $(LIB_DIR)/libft.a

# -------------------------------------------------
#  Build rules
# -------------------------------------------------
all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# -------------------------------------------------
#  Compile each .c → .o
# -------------------------------------------------
$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	rmdir $(OBJS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all bonus clean fclean re
