.PHONY: all clean

CC = gcc

CFLAGS = -Wall -Wextra

TARGETS = mycat mygrep

SOURCES = mycat.c mygrep.c

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGETS)

mycat: mycat.o
	$(CC) $(CFLAGS) -o mycat mycat.o

mygrep: mygrep.o
	$(CC) $(CFLAGS) -o mygrep mygrep.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGETS)
