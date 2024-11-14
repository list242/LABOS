#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#ifdef __unix__
#include <unistd.h>


#define BUFFER_SIZE 4096

// Функция добавления файла в архив
int add_file_to_archive(const char *archive_name, const char *filename) {
    int archive_fd = open(archive_name, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (archive_fd < 0) {
        perror("Error opening archive");
        return -1;
    }

    int file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening file");
        close(archive_fd);
        return -1;
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0) {
        perror("Error getting file stats");
        close(file_fd);
        close(archive_fd);
        return -1;
    }

    int name_len = strlen(filename);
    write(archive_fd, &name_len, sizeof(name_len));  // Записываем длину имени файла
    write(archive_fd, filename, name_len);           // Записываем имя файла
    write(archive_fd, &file_stat.st_size, sizeof(file_stat.st_size));  // Записываем размер файла
    write(archive_fd, &file_stat, sizeof(file_stat)); // Записываем атрибуты файла

    // Копирование содержимого файла
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        write(archive_fd, buffer, bytes_read);
    }

    close(file_fd);
    close(archive_fd);
    return 0;
}

// Функция извлечения файла из архива
int extract_file_from_archive(const char *archive_name, const char *filename, int delete_after) {
    int archive_fd = open(archive_name, O_RDWR);
    if (archive_fd < 0) {
        perror("Error opening archive");
        return -1;
    }

    int temp_fd = open("temp_archive", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd < 0) {
        perror("Error creating temporary archive");
        close(archive_fd);
        return -1;
    }

    int name_len;
    char name_buffer[256];
    off_t file_size;
    struct stat file_stat;
    int found = 0;

    while (read(archive_fd, &name_len, sizeof(name_len)) == sizeof(name_len)) {
        read(archive_fd, name_buffer, name_len);
        name_buffer[name_len] = '\0';
        read(archive_fd, &file_size, sizeof(file_size));
        read(archive_fd, &file_stat, sizeof(file_stat));

        if (strcmp(name_buffer, filename) == 0) {
            found = 1;
            int output_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, file_stat.st_mode);
            if (output_fd < 0) {
                perror("Error creating output file");
                close(archive_fd);
                close(temp_fd);
                return -1;
            }

            // Копируем содержимое файла
            char buffer[BUFFER_SIZE];
            off_t remaining = file_size;
            while (remaining > 0) {
                ssize_t bytes_to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
                ssize_t bytes_read = read(archive_fd, buffer, bytes_to_read);
                if (bytes_read <= 0) break;
                write(output_fd, buffer, bytes_read);
                remaining -= bytes_read;
            }

            close(output_fd);
            if (!delete_after) {
                write(temp_fd, &name_len, sizeof(name_len));
                write(temp_fd, name_buffer, name_len);
                write(temp_fd, &file_size, sizeof(file_size));
                write(temp_fd, &file_stat, sizeof(file_stat));
            }
        } else {
            write(temp_fd, &name_len, sizeof(name_len));
            write(temp_fd, name_buffer, name_len);
            write(temp_fd, &file_size, sizeof(file_size));
            write(temp_fd, &file_stat, sizeof(file_stat));

            char buffer[BUFFER_SIZE];
            off_t remaining = file_size;
            while (remaining > 0) {
                ssize_t bytes_to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
                ssize_t bytes_read = read(archive_fd, buffer, bytes_to_read);
                write(temp_fd, buffer, bytes_read);
                remaining -= bytes_read;
            }
        }
    }

    close(archive_fd);
    close(temp_fd);

    if (found) {
        rename("temp_archive", archive_name);
    } else {
        remove("temp_archive");
        printf("File %s not found in archive\n", filename);
        return -1;
    }

    return 0;
}

// Функция отображения информации об архиве
int show_archive_stat(const char *archive_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd < 0) {
        perror("Error opening archive");
        return -1;
    }

    int name_len;
    char name_buffer[256];
    off_t file_size;
    struct stat file_stat;

    printf("Archive contents:\n");
    while (read(archive_fd, &name_len, sizeof(name_len)) == sizeof(name_len)) {
        read(archive_fd, name_buffer, name_len);
        name_buffer[name_len] = '\0';
        read(archive_fd, &file_size, sizeof(file_size));
        read(archive_fd, &file_stat, sizeof(file_stat));

        printf("File: %s, Size: %ld bytes\n", name_buffer, file_size);
    }

    close(archive_fd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    const char *archive_name = argv[1];
    const char *command = argv[2];

    if (strcmp(command, "-i") == 0 || strcmp(command, "--input") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Invalid arguments\n");
            return 1;
        }
        return add_file_to_archive(archive_name, argv[3]);
    } else if (strcmp(command, "-e") == 0 || strcmp(command, "--extract") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Invalid arguments\n");
            return 1;
        }
        return extract_file_from_archive(archive_name, argv[3], 1);
    } else if (strcmp(command, "-s") == 0 || strcmp(command, "--stat") == 0) {
        return show_archive_stat(archive_name);
    } else {
        fprintf(stderr, "Invalid command\n");
        return 1;
    }
}
#endif