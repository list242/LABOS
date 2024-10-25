.PHONY: all clean

# Компилятор
CC = gcc

# Флаги компилятора
CFLAGS = -Wall -Wextra

# Исполняемые файлы
TARGETS = mycat mygrep

# Исходные файлы
SOURCES = mycat.c mygrep.c

# Объектные файлы
OBJECTS = $(SOURCES:.c=.o)

# Правило по умолчанию для сборки
all: $(TARGETS)

# Правила для сборки mycat
mycat: mycat.o
	$(CC) $(CFLAGS) -o mycat mycat.o

# Правила для сборки mygrep
mygrep: mygrep.o
	$(CC) $(CFLAGS) -o mygrep mygrep.o

# Правила для компиляции .c в .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Правило для очистки
clean:
	rm -f $(OBJECTS) $(TARGETS)
