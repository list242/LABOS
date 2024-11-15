#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Проверка, является ли строка пустой
int is_empty_line(const char *line) {
    while (*line) {
        if (!isspace((unsigned char)*line)) {
            return 0;  // Строка не пустая
        }
        line++;
    }
    return 1;  // Строка пустая
}

void print_line(char *line, int number_all, int number_nonempty, int show_ends, int *line_number_ptr) {
    size_t len = strlen(line);

    // Убираем символ новой строки в конце строки
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }

    // Нумерация строк в зависимости от флагов
    if (number_nonempty && !is_empty_line(line)) {
        printf("%6d\t", (*line_number_ptr)++);
    } else if (number_all) {
        printf("%6d\t", (*line_number_ptr)++);
    }

    // Печать строки с символом конца строки, если требуется
    if (show_ends) {
        printf("%s$\n", line);  // Добавляем символ $ в конце строки
    } else {
        printf("%s\n", line);  // Просто выводим строку без изменений
    }
}

void process_file(FILE *file, int number_all, int number_nonempty, int show_ends) {
    char line[4096];
    int line_number = 1;

    // Чтение строк из файла
    while (fgets(line, sizeof(line), file)) {
        if (number_nonempty && is_empty_line(line)) {
            // Если флаг -b и строка пустая, пропускаем нумерацию
            printf("\n");
        } else {
            print_line(line, number_all, number_nonempty, show_ends, &line_number);
        }
    }
}

int main(int argc, char *argv[]) {
    int number_all = 0, number_nonempty = 0, show_ends = 0;
    int file_count = 0;
    char *files[argc];  // Массив для хранения имен файлов

    // Обработка аргументов командной строки
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            // Обработка флагов
            for (int j = 1; argv[i][j] != '\0'; ++j) {
                switch (argv[i][j]) {
                    case 'n':
                        number_all = 1;  // Нумеровать все строки
                        break;
                    case 'b':
                        number_nonempty = 1;  // Нумеровать только непустые строки
                        break;
                    case 'E':
                        show_ends = 1;  // Печать $ в конце строки
                        break;
                    default:
                        fprintf(stderr, "Неизвестный параметр: -%c\n", argv[i][j]);
                        return 1;
                }
            }
        } else {
            // Если это не флаг, то это имя файла
            files[file_count++] = argv[i];
        }
    }

    // Если файлы не указаны, читаем из стандартного ввода
    if (file_count == 0) {
        process_file(stdin, number_all, number_nonempty, show_ends);
    } else {
        // Обработка файлов
        for (int i = 0; i < file_count; ++i) {
            FILE *file = fopen(files[i], "r");
            if (!file) {
                perror("Ошибка при открытии файла");
                continue;
            }
            process_file(file, number_all, number_nonempty, show_ends);
            fclose(file);
        }
    }

    return 0;
}
