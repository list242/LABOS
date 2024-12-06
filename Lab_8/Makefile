.PHONY: all clean run

all: sync_threads

sync_threads: sync_threads.c
	gcc sync_threads.c -o sync_threads -Wall -Wextra

run: sync_threads
	./sync_threads

clean:
	rm -rf *.o sync_threads
