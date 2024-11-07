#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define USER_SHIFT 6
#define GROUP_SHIFT 3
#define OTHER_SHIFT 0

int change_mode(const char *mode_str, const char *file) {
    struct stat st;
    if (stat(file, &st) == -1) {
        perror("Ошибка получения информации о файле");
        return -1;
    }

    mode_t new_mode = st.st_mode;

    if (isdigit(mode_str[0])) {
        new_mode = strtol(mode_str, NULL, 8);
        if (chmod(file, new_mode) == -1) {
            perror("Ошибка изменения прав доступа");
            return -1;
        }
    } else {
        char operation = ' ';
        int groupShift = -1; 
        int permissionShift = -1;

        for (size_t i = 0; i < strlen(mode_str); ++i) {
            switch (mode_str[i]) {
                case 'u':
                    groupShift = USER_SHIFT;
                    break;
                case 'g':
                    groupShift = GROUP_SHIFT;
                    break;
                case 'o':
                    groupShift = OTHER_SHIFT;
                    break;
                case '+':
                case '-':
                    operation = mode_str[i];
                    break;
                case 'r':
                    permissionShift = 2;
                    break;
                case 'w':
                    permissionShift = 1;
                    break;
                case 'x':
                    permissionShift = 0;
                    break;
                case ',':
                    groupShift = -1;
                    permissionShift = -1;
                    break;
            }
            if (permissionShift == -1 || operation == ' ') continue;
            if (groupShift == -1) {
                new_mode = operation == '+' ? (new_mode | (1 << (USER_SHIFT + permissionShift)) | (1 << (GROUP_SHIFT + permissionShift)) | (1 << (OTHER_SHIFT + permissionShift))) :
                                              (new_mode & ~((1 << (USER_SHIFT + permissionShift)) | (1 << (GROUP_SHIFT + permissionShift)) | (1 << (OTHER_SHIFT + permissionShift))));
            } else {
                new_mode = operation == '+' ? (new_mode | (1 << (groupShift + permissionShift))) : 
                                              (new_mode & ~(1 << (groupShift + permissionShift)));
            }
        }

        if (chmod(file, new_mode) == -1) {
            perror("Ошибка изменения прав доступа");
            return -1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Использование: %s <режим> <файл>\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 2; i < argc; ++i) {
        if (change_mode(argv[1], argv[i]) == -1) {
            return EXIT_FAILURE;
        }
    }

    printf("Права доступа успешно изменены.\n");
    return EXIT_SUCCESS;
}
