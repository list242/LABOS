.PHONY: all clean run

all: semaphore_sync

semaphore_sync: src/semaphore_sync.c
	gcc src/semaphore_sync.c -o semaphore_sync -Wall -Wextra -D__unix__

run: semaphore_sync
	./semaphore_sync producer & ./semaphore_sync consumer

clean:
	rm -rf *.o semaphore_sync
