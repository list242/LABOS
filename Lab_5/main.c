#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_FILENAME 256
#define HEADER_SIZE (sizeof(struct file_entry))

struct file_entry {
    char filename[MAX_FILENAME];
    size_t size;
    mode_t mode;
};

void archive_file(int archive_fd, const char *filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        perror("Ошибка при получении информации о файле");
        return;
    }

    struct file_entry entry;
    lseek(archive_fd, 0, SEEK_SET);
    while (read(archive_fd, &entry, HEADER_SIZE) > 0) {
        if (strcmp(entry.filename, filename) == 0) {
            fprintf(stderr, "Файл %s уже существует в архиве. Добавление отменено\n", filename);
            return;
        }
        lseek(archive_fd, entry.size, SEEK_CUR);
    }

    strncpy(entry.filename, filename, MAX_FILENAME);
    entry.size = st.st_size;
    entry.mode = st.st_mode;

    write(archive_fd, &entry, HEADER_SIZE);

    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        perror("Ошибка при открытии файла");
        return;
    }

    char *buffer = malloc(entry.size);
    read(file_fd, buffer, entry.size);
    write(archive_fd, buffer, entry.size);

    free(buffer);
    close(file_fd);
}

void extract_and_remove_file(const char *arch_name, const char *filename) {
    int archive_fd = open(arch_name, O_RDWR);
    if (archive_fd == -1) {
        perror("Ошибка при открытии архива");
        return;
    }

    int temp_fd = open("temp_archive", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (temp_fd == -1) {
        perror("Ошибка при создании временного архива");
        close(archive_fd);
        return;
    }

    struct file_entry entry;
    int found = 0;

    while (read(archive_fd, &entry, HEADER_SIZE) > 0) {
        if (strcmp(entry.filename, filename) == 0) {
            found = 1;
            char *buffer = malloc(entry.size);
            read(archive_fd, buffer, entry.size);
            int file_fd = open(entry.filename, O_WRONLY | O_CREAT | O_TRUNC, entry.mode);
            write(file_fd, buffer, entry.size);
            close(file_fd);
            free(buffer);
        } else {
            write(temp_fd, &entry, HEADER_SIZE);
            char *buffer = malloc(entry.size);
            read(archive_fd, buffer, entry.size);
            write(temp_fd, buffer, entry.size);
            free(buffer);
        }
    }

    if (!found) {
        fprintf(stderr, "Файл %s не найден в архиве\n", filename);
    }

    close(temp_fd);
    close(archive_fd);
    rename("temp_archive", arch_name);
}

void show_stat(int archive_fd) {
    struct file_entry entry;
    while (read(archive_fd, &entry, HEADER_SIZE) > 0) {
        printf("Файл: %s Размер: %zu Режим: %o\n", entry.filename, entry.size, entry.mode);
        lseek(archive_fd, entry.size, SEEK_CUR);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s arch_name -i file1 | -e file1 | -s | -h\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)){
        printf("\nИспользование: %s arch_name [опции]\n", argv[0]);
        printf("\nОпции:\n");
        printf("  -i file1 ...     Добавить файлы в архив\n");
        printf("  -e file1         Извлечь файл из архива и удалить его\n");
        printf("  -s               Показать состояние архива\n");
        printf("  -h               Показать это сообщение\n");
        return EXIT_SUCCESS;
    }

    const char *arch_name = argv[1];
    int archive_fd = open(arch_name, O_RDWR | O_CREAT | O_APPEND, 0666);

    if (archive_fd == -1) {
        perror("Ошибка при открытии архива");
        return EXIT_FAILURE;
    }

    if (argc == 3 || argc == 4) {
        if (strcmp(argv[2], "-i") == 0 || strcmp(argv[2], "--input") == 0) {
            if (argc < 4) {
                fprintf(stderr, "Не указано имя файла для добавления\n");
                close(archive_fd);
                return EXIT_FAILURE;
            }

            for (int i = 3; i < argc; i++) {
                archive_file(archive_fd, argv[i]);
            }

        } else if (strcmp(argv[2], "-e") == 0 || strcmp(argv[2], "--extract") == 0) {
            if (argc < 4) {
                fprintf(stderr, "Не указано имя файла для извлечения\n");
                close(archive_fd);
                return EXIT_FAILURE;
            }
            extract_and_remove_file(arch_name, argv[3]);

        } else if (strcmp(argv[2], "-s") == 0 || strcmp(argv[2], "--stat") == 0) {
            if (argc == 3) {
                show_stat(archive_fd);
            } else {
                fprintf(stderr, "Неправильно указана опция\n");
                close(archive_fd);
                return EXIT_FAILURE;
            }

        } else {
            fprintf(stderr, "Неизвестная опция: %s\n", argv[2]);
            close(archive_fd);
            return EXIT_FAILURE;
        }

    } else {
        fprintf(stderr, "Не указана опция или указана неправильно\n");
        close(archive_fd);
        return EXIT_FAILURE;
    }

    close(archive_fd);
    return EXIT_SUCCESS;
}
