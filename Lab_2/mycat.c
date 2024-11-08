#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int is_empty_line(const char *line) {
    while (*line) {
        if (!isspace((unsigned char)*line)) {
            return 0;
        }
        line++;
    }
    return 1;
}

void trim(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

void print_file(FILE *file, int number_all, int number_nonempty, int show_ends, int *line_number_ptr, int show_end_at_start) {
    char line[4096];

    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        trim(line);
        int is_empty = is_empty_line(line);

        if (number_nonempty && !is_empty) {
            printf("%6d\t", (*line_number_ptr)++);
        } else if (number_all) {
            printf("%6d\t", (*line_number_ptr)++);
        }

        if (show_ends) {
            if (!is_empty) {
                if (show_end_at_start) {
                    printf("$%s\n", line);
                } else {
                    printf("%s$\n", line);
                }
            } else {
                printf("$\n");
            }
        } else {
            printf("%s\n", line);
        }
    }
}

int main(int argc, char *argv[]) {
    int number_all = 0, number_nonempty = 0, show_ends = 0;
    int line_number = 1;
    int files_provided = 0;
    int arg_index = 1;
    int show_end_at_start = 0;

    for (; arg_index < argc; ++arg_index) {
        if (argv[arg_index][0] == '-') {
            for (int j = 1; argv[arg_index][j] != '\0'; ++j) {
                switch (argv[arg_index][j]) {
                    case 'n':
                        number_all = 1;
                        break;
                    case 'b':
                        number_nonempty = 1;
                        break;
                    case 'E':
                        show_ends = 1;
                        break;
                    default:
                        fprintf(stderr, "Неизвестный параметр: -%c\n", argv[arg_index][j]);
                        return 1;
                }
            }
        } else {
            files_provided = 1;
            break;
        }
    }

    if (number_nonempty) {
        number_all = 0;
    }

    if (show_ends && number_nonempty == 0) {
        show_end_at_start = 1;
    }

    if (!files_provided) {
        print_file(stdin, number_all, number_nonempty, show_ends, &line_number, show_end_at_start);
    } else {
        for (int i = arg_index; i < argc; ++i) {
            FILE *file = fopen(argv[i], "r");
            if (!file) {
                perror("Ошибка при открытии файла");
                exit(1);
            }
            print_file(file, number_all, number_nonempty, show_ends, &line_number, show_end_at_start);
            fclose(file);
        }
    }

    return 0;
}
