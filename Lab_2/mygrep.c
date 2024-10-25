#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 1024

// Функция для поиска шаблона в строке
int match_pattern(const char *line, const char *pattern) {
    return strstr(line, pattern) != NULL;
}

// Основная функция для поиска шаблона в файле или стандартном вводе
void grep_pattern(FILE *file, const char *pattern) {
    char buffer[MAX_LINE_LEN];
    // Чтение файла построчно и вывод строк с совпадением pattern
    while (fgets(buffer, sizeof(buffer), file)) {
        if (match_pattern(buffer, pattern)) {
            printf("%s", buffer);  // Выводим каждую подходящую строку в терминал
            fflush(stdout);        // Сброс буфера для немедленного вывода
        }
    }
}

// Функция для открытия файла или использования стандартного ввода
FILE* open_file(const char *filename) {
    FILE *file = filename ? fopen(filename, "r") : stdin;
    if (!file) {
        perror("Error opening file");
        exit(1);
    }
    return file;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pattern> [file]\n", argv[0]);
        return 1;
    }

    const char *pattern = argv[1];
    const char *filename = argc > 2 ? argv[2] : NULL;

    FILE *file = open_file(filename);
    grep_pattern(file, pattern);

    // Закрываем файл, только если он не является stdin
    if (file != stdin) {
        fclose(file);
    }

    return 0;
}
