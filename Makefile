# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/20 19:26:08 by slenti            #+#    #+#              #
#    Updated: 2026/11/02 22:21:05 by slenti           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# -------------------------------------------------
#  Variables
# -------------------------------------------------
CC = cc
MAX_CODERS ?= 256
ifeq ($(shell test $(MAX_CODERS) -gt 2147483647 && echo yes || echo no),yes)
$(error MAX_CODERS exceeds INT_MAX)
endif
ifeq ($(shell test $(MAX_CODERS) -le 0 && echo yes || echo no),yes)
$(error MAX_CODERS must be greater than 0)
endif
CFLAGS = -Wall -Wextra -Werror -pthread -I$(INC_DIR) -DMAX_CODERS=$(MAX_CODERS)
RM = rm -f

NAME = codexion
SRCS_DIR := srcs
OBJS_DIR := build
INC_DIR := includes

SRCS := main.c error.c validate.c simulation.c monitor.c coders.c dongles.c \
		logger.c coder_routine.c adquire1.c adquire2.c heap.c

SRCS_FULL := $(addprefix $(SRCS_DIR)/,$(SRCS))
OBJS := $(patsubst $(SRCS_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRCS_FULL))

# -------------------------------------------------
#  Build rules
# -------------------------------------------------
all: $(NAME)

$(OBJS_DIR):
	mkdir -p $@

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# -------------------------------------------------
#  Compile each .c → .o
# -------------------------------------------------
$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c | $(OBJS_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	rmdir $(OBJS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all bonus clean fclean re
