# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11

all: my_pipe my_fifo

my_pipe: my_pipe.c
	$(CC) $(CFLAGS) -o my_pipe my_pipe.c

my_fifo: my_fifo.c
	$(CC) $(CFLAGS) -o my_fifo my_fifo.c


clean:
	rm -f my_pipe my_fifo

.PHONY: all clean
