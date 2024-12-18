CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = arch

all: $(TARGET)

$(TARGET): arch.c
	$(CC) $(CFLAGS) -o $(TARGET) arch.c

clean:
	rm -f $(TARGET) *.o
