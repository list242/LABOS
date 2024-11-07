#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_LEN 1024

void print_lines(FILE *file, int show_nonempty_line_num, int show_all_line_num, int show_ends, int *line_number_ptr) {
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        int is_empty = (line[0] == '\n' || line[0] == '\0');

        if (show_nonempty_line_num) {
            if (!is_empty) {
                printf("%6d\t", (*line_number_ptr)++);
            }
        } else if (show_all_line_num) {
            printf("%6d\t", (*line_number_ptr)++);
        }

        size_t len = strlen(line);
        if (show_ends) {
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
                printf("%s$\n", line);
            } else {
                printf("%s$", line);
            }
        } else {
            fputs(line, stdout);
        }
    }
}

int main(int argc, char *argv[]) {
    int show_nonempty_line_num = 0; // Нумеровать только непустые строки
    int show_all_line_num = 0; // Нумеровать все строки
    int show_ends = 0; // Добавлять $ в конец строки
    int line_number = 1;
    int opt;

    // Обработка флагов
    while ((opt = getopt(argc, argv, "nEb")) != -1) {
        switch (opt) {
            case 'b':
                show_nonempty_line_num = 1;
                break;
            case 'n':
                show_all_line_num = 1;
                break;
            case 'E':
                show_ends = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-n|-b|-E] [file...]\n", argv[0]);
                return 1;
        }
    }

    if (show_nonempty_line_num) {
        show_all_line_num = 0;
    }

    if (optind >= argc) {
        print_lines(stdin, show_nonempty_line_num, show_all_line_num, show_ends, &line_number);
    } else {
        for (int i = optind; i < argc; i++) {
            FILE *file = fopen(argv[i], "r");
            if (!file) {
                perror("Ошибка при открытии файла");
                continue;
            }

            if (argc - optind > 1) {
                printf("==> %s <==\n", argv[i]);
            }

            print_lines(file, show_nonempty_line_num, show_all_line_num, show_ends, &line_number);
            fclose(file);
        }
    }

    return 0;
}
