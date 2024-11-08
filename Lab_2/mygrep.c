#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void grep_file(const char *pattern, FILE *file) {
    char line[4096];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, pattern)) {
            fputs(line, stdout);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s pattern [file...]\n", argv[0]);
        return 1;
    }
    const char *pattern = argv[1];
    if (argc == 2) {
        grep_file(pattern, stdin);
    } else {
        for (int i = 2; i < argc; ++i) {
            FILE *file = fopen(argv[i], "r");
            if (!file) {
                perror("Ошибка при открытии файла");
                continue;
            }
            grep_file(pattern, file);
            fclose(file);
        }
    }
    return 0;
}

