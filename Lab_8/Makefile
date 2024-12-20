EXECUTABLE = mymutex

SOURCES = main.c

CFLAGS = -g -Wall -lpthread

$(EXECUTABLE): $(SOURCES)
	gcc $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)
