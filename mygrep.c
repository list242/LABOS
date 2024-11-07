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
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pattern> [file...]\n", argv[0]);
        return 1;
    }

    const char *pattern = argv[1];

    if (argc == 2) { // Если передан только шаблон, читаем из стандартного ввода
        grep_pattern(stdin, pattern);
    } else { // Иначе обрабатываем каждый файл из списка аргументов
        for (int i = 2; i < argc; ++i) {
            FILE *file = fopen(argv[i], "r");
            if (!file) {
                perror("Ошибка при открытии файла");
                continue; // Переход к следующему файлу при ошибке
            }

            // Печатаем заголовок перед содержимым, если несколько файлов
            if (argc > 3) {
                printf("==> %s <==\n", argv[i]);
            }

            grep_pattern(file, pattern);
            fclose(file);
        }
    }

    return 0;
}
