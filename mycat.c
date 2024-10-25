#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_LEN 1024

void print_lines(FILE *file, int show_nonempty_line_num, int show_all_line_num, int show_ends) {
    char line[1024];
    int line_num = 1; // Общий номер строки
    int non_empty_line_num = 1; // Номер непустой строки

    while (fgets(line, sizeof(line), file)) {
        // Убираем символ новой строки, если он есть
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0'; // Заменяем на нуль-терминатор
        }

        // Проверяем, является ли строка пустой
        if (strlen(line) == 0) {
            continue; // Пропускаем пустые строки
        }

        // Если нужно показать символ конца строки
        if (show_ends) {
    // Проверка на пустую строку
    if (strlen(line) > 0 && line[strlen(line) - 1] != '\n') {
        strcat(line, "$");
    }
}


        // Выводим строки с номерами
        if (show_nonempty_line_num) {
            printf("%6d\t%s\n", non_empty_line_num++, line); // Для непустых строк
        } else if (show_all_line_num) {
            printf("%6d\t%s\n", line_num++, line); // Для всех строк
        } else {
            printf("%s\n", line); // Только выводим строку
        }
        line_num++; // Увеличиваем общий номер строки
    }
}


int main(int argc, char *argv[]) {
    int show_nonempty_line_num = 0; // Нумеровать только непустые строки
    int show_all_line_num = 0; // Нумеровать все строки
    int show_ends = 0; // Добавлять $ в конец строки
    char *filename = NULL; // Имя файла
    int opt;

    // Обработка флагов
    while ((opt = getopt(argc, argv, "nEb")) != -1) {
        switch (opt) {
            case 'b':
                show_nonempty_line_num = 1; // Включаем нумерацию только непустых строк
                break;
            case 'n':
                show_all_line_num = 1; // Включаем нумерацию всех строк
                break;
            case 'E':
                show_ends = 1; // Включаем добавление символа $
                break;
            default:
                fprintf(stderr, "Usage: %s [-n|-b|-E] [file]\n", argv[0]);
                return 1;
        }
    }

    // Получаем имя файла, если оно указано
    if (optind < argc) {
        filename = argv[optind];
    }

    // Открываем файл или стандартный ввод
    FILE *file = filename ? fopen(filename, "r") : stdin;
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    // Печатаем строки с учетом флагов
    print_lines(file, show_nonempty_line_num, show_all_line_num, show_ends);

    // Закрываем файл, если это не stdin
    if (file != stdin) {
        fclose(file);
    }

    return 0;
}
