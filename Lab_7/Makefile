CC = gcc
CFLAGS = -Wall -g

TARGETS = sender recevier

all: $(TARGETS)

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c

receiver: receiver.c
	$(CC) $(CFLAGS) -o recevier recevier.c

clean:
	rm -f $(TARGETS)

.PHONY: all clean
