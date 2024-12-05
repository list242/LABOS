.PHONY: all clean run

SRC = main.c
OUT = my_program

CC = gcc
CFLAGS = -Wall -Wextra

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: $(OUT)
	./$(OUT)

clean:
	rm -rf *.o $(OUT)
