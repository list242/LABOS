.PHONY: all clean run_sender run_receiver

# Переменные
CC = gcc
CFLAGS = -Wall -Wextra

# Программы
SENDER = sender
RECEIVER = receiver

# Исходники
SENDER_SRC = sender.c
RECEIVER_SRC = receiver.c

# Бинарники
SENDER_BIN = sender
RECEIVER_BIN = receiver

# Цель по умолчанию
all: $(SENDER_BIN) $(RECEIVER_BIN)

# Правило для сборки sender
$(SENDER_BIN): $(SENDER_SRC)
	$(CC) $(SENDER_SRC) -o $(SENDER_BIN) $(CFLAGS)

# Правило для сборки receiver
$(RECEIVER_BIN): $(RECEIVER_SRC)
	$(CC) $(RECEIVER_SRC) -o $(RECEIVER_BIN) $(CFLAGS)

# Запуск sender
run_sender: $(SENDER_BIN)
	./$(SENDER_BIN)

# Запуск receiver
run_receiver: $(RECEIVER_BIN)
	./$(RECEIVER_BIN)

# Очистка
clean:
	rm -rf *.o $(SENDER_BIN) $(RECEIVER_BIN)
