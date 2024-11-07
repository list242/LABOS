#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define ARCHIVE_MAGIC "ARCHIVE"  // Магическое слово для идентификации архива
#define ARCHIVE_MAGIC_SIZE sizeof(ARCHIVE_MAGIC)

// Структура для записи файла в архив
typedef struct {
    char filename[256];
    size_t size;
    mode_t mode;
} ArchiveEntry;

// Функция для создания архива
void create_archive(const char *archive_name, const char *file_name) {
    int archive_fd = open(archive_name, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (archive_fd == -1) {
        perror("Ошибка открытия архива");
        return;
    }

    // Запись магического слова в архив
    write(archive_fd, ARCHIVE_MAGIC, ARCHIVE_MAGIC_SIZE);

    // Получение информации о файле
    struct stat file_info;
    if (stat(file_name, &file_info) == -1) {
        perror("Ошибка получения информации о файле");
        close(archive_fd);
        return;
    }

    // Создание записи в архиве
    ArchiveEntry entry;
    strncpy(entry.filename, file_name, sizeof(entry.filename) - 1);
    entry.filename[sizeof(entry.filename) - 1] = '\0';
    entry.size = file_info.st_size;
    entry.mode = file_info.st_mode;

    // Запись записи в архив
    write(archive_fd, &entry, sizeof(entry));

    // Чтение содержимого файла и запись в архив
    int file_fd = open(file_name, O_RDONLY);
    if (file_fd == -1) {
        perror("Ошибка открытия файла для чтения");
        close(archive_fd);
        return;
    }

    char *buffer = malloc(entry.size);
    read(file_fd, buffer, entry.size);
    write(archive_fd, buffer, entry.size);

    free(buffer);
    close(file_fd);
    close(archive_fd);
    printf("Файл '%s' добавлен в архив '%s'.\n", file_name, archive_name);
}

// Функция для извлечения файла из архива
void extract_file(const char *archive_name, const char *file_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1) {
        perror("Ошибка открытия архива");
        return;
    }

    char magic[ARCHIVE_MAGIC_SIZE];
    read(archive_fd, magic, ARCHIVE_MAGIC_SIZE);
    if (strncmp(magic, ARCHIVE_MAGIC, ARCHIVE_MAGIC_SIZE) != 0) {
        fprintf(stderr, "Это не архив.\n");
        close(archive_fd);
        return;
    }

    ArchiveEntry entry;
    while (read(archive_fd, &entry, sizeof(entry)) > 0) {
        if (strcmp(entry.filename, file_name) == 0) {
            char *buffer = malloc(entry.size);
            read(archive_fd, buffer, entry.size);

            // Восстановление файла с сохранением атрибутов
            int file_fd = open(entry.filename, O_WRONLY | O_CREAT | O_TRUNC, entry.mode);
            if (file_fd == -1) {
                perror("Ошибка открытия файла для записи");
                free(buffer);
                close(archive_fd);
                return;
            }
            write(file_fd, buffer, entry.size);
            free(buffer);
            close(file_fd);

            printf("Файл '%s' извлечен из архива '%s'.\n", file_name, archive_name);
            close(archive_fd);
            return;
        }
    }

    printf("Файл '%s' не найден в архиве '%s'.\n", file_name, archive_name);
    close(archive_fd);
}

// Функция для отображения состояния архива
void display_archive(const char *archive_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd == -1) {
        perror("Ошибка открытия архива");
        return;
    }

    char magic[ARCHIVE_MAGIC_SIZE];
    read(archive_fd, magic, ARCHIVE_MAGIC_SIZE);
    if (strncmp(magic, ARCHIVE_MAGIC, ARCHIVE_MAGIC_SIZE) != 0) {
        fprintf(stderr, "Это не архив.\n");
        close(archive_fd);
        return;
    }

    ArchiveEntry entry;
    while (read(archive_fd, &entry, sizeof(entry)) > 0) {
        printf("Файл: %s, Размер: %zu байт, Права: %o\n", entry.filename, entry.size, entry.mode);
        lseek(archive_fd, entry.size, SEEK_CUR);  // Пропустить содержимое файла
    }

    close(archive_fd);
}

// Функция для обработки аргументов командной строки
void parse_arguments(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Неверный параметр.\n");
        exit(EXIT_FAILURE);
    }

    const char *archive_name = argv[1];
    const char *option = argv[2];
    const char *file_name = argv[3];

    if (strcmp(option, "-i") == 0 || strcmp(option, "--input") == 0) {
        create_archive(archive_name, file_name);
    } else if (strcmp(option, "-e") == 0 || strcmp(option, "--extract") == 0) {
        extract_file(archive_name, file_name);
    } else if (strcmp(option, "-s") == 0 || strcmp(option, "--stat") == 0) {
        display_archive(archive_name);
    } else {
        fprintf(stderr, "Неверный параметр.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    parse_arguments(argc, argv);
    return 0;
}
