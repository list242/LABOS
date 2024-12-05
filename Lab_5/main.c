
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
#define MAX_FILENAME_LEN 256

typedef struct {
    mode_t st_mode;
    uid_t st_uid;
    gid_t st_gid;
    off_t st_size;
    time_t st_mtime;
} file_meta_t;

// Функция добавления файла в архив
int add_file_to_archive(const char *archive_name, const char *filename) {
    if (strlen(filename) > MAX_FILENAME_LEN) {
        fprintf(stderr, "Filename too long: %s\n", filename);
        return -1;
    }

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
    if (write(archive_fd, &name_len, sizeof(name_len)) != sizeof(name_len) ||
        write(archive_fd, filename, name_len) != name_len) {
        perror("Error writing filename to archive");
        close(file_fd);
        close(archive_fd);
        return -1;
    }

    file_meta_t meta = {
        .st_mode = file_stat.st_mode,
        .st_uid = file_stat.st_uid,
        .st_gid = file_stat.st_gid,
        .st_size = file_stat.st_size,
        .st_mtime = file_stat.st_mtime
    };

    if (write(archive_fd, &meta, sizeof(meta)) != sizeof(meta)) {
        perror("Error writing file metadata to archive");
        close(file_fd);
        close(archive_fd);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        if (write(archive_fd, buffer, bytes_read) != bytes_read) {
            perror("Error writing file content to archive");
            close(file_fd);
            close(archive_fd);
            return -1;
        }
    }

    if (bytes_read < 0) {
        perror("Error reading file content");
    }

    close(file_fd);
    close(archive_fd);
    return 0;
}

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
    char name_buffer[MAX_FILENAME_LEN];
    file_meta_t meta;
    int found = 0;

    while (read(archive_fd, &name_len, sizeof(name_len)) == sizeof(name_len)) {
        if (name_len >= MAX_FILENAME_LEN ||
            read(archive_fd, name_buffer, name_len) != name_len) {
            fprintf(stderr, "Error reading filename from archive\n");
            break;
        }
        name_buffer[name_len] = '\0';

        if (read(archive_fd, &meta, sizeof(meta)) != sizeof(meta)) {
            fprintf(stderr, "Error reading metadata from archive\n");
            break;
        }

        if (strcmp(name_buffer, filename) == 0) {
            found = 1;
            int output_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, meta.st_mode);
            if (output_fd < 0) {
                perror("Error creating output file");
                close(archive_fd);
                close(temp_fd);
                return -1;
            }

            char buffer[BUFFER_SIZE];
            off_t remaining = meta.st_size;
            while (remaining > 0) {
                ssize_t bytes_to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
                ssize_t bytes_read = read(archive_fd, buffer, bytes_to_read);
                if (bytes_read <= 0) break;
                if (write(output_fd, buffer, bytes_read) != bytes_read) {
                    perror("Error writing to output file");
                    close(output_fd);
                    close(archive_fd);
                    close(temp_fd);
                    return -1;
                }
                remaining -= bytes_read;
            }

            close(output_fd);
            if (!delete_after) {
                write(temp_fd, &name_len, sizeof(name_len));
                write(temp_fd, name_buffer, name_len);
                write(temp_fd, &meta, sizeof(meta));
            }
        } else {
            write(temp_fd, &name_len, sizeof(name_len));
            write(temp_fd, name_buffer, name_len);
            write(temp_fd, &meta, sizeof(meta));

            char buffer[BUFFER_SIZE];
            off_t remaining = meta.st_size;
            while (remaining > 0) {
                ssize_t bytes_to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
                ssize_t bytes_read = read(archive_fd, buffer, bytes_to_read);
                if (bytes_read <= 0) break;
                write(temp_fd, buffer, bytes_read);
                remaining -= bytes_read;
            }
        }
    }

    close(archive_fd);
    close(temp_fd);

    if (found) {
        if (rename("temp_archive", archive_name) < 0) {
            perror("Error renaming temporary archive");
            return -1;
        }
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
    char name_buffer[MAX_FILENAME_LEN];
    file_meta_t meta;

    printf("Archive contents:\n");
    while (read(archive_fd, &name_len, sizeof(name_len)) == sizeof(name_len)) {
        if (name_len >= MAX_FILENAME_LEN ||
            read(archive_fd, name_buffer, name_len) != name_len) {
            fprintf(stderr, "Error reading filename from archive\n");
            break;
        }
        name_buffer[name_len] = '\0';

        if (read(archive_fd, &meta, sizeof(meta)) != sizeof(meta)) {
            fprintf(stderr, "Error reading metadata from archive\n");
            break;
        }

        printf("File: %s, Size: %ld bytes\n", name_buffer, meta.st_size);
    }

    close(archive_fd);
    return 0;
}

void print_help() {
    printf("Usage: ./archiver archive_name [options]\n");
    printf("Options:\n");
    printf("  -i, --input FILE   Add FILE to the archive\n");
    printf("  -e, --extract FILE Extract FILE from the archive and remove it\n");
    printf("  -s, --stat         Show archive contents\n");
    printf("  -h, --help         Show this help message\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_help();
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
    } else if (strcmp(command, "-h") == 0 || strcmp(command, "--help") == 0) {
        print_help();
        return 0;
    } else {
        fprintf(stderr, "Invalid command\n");
        print_help();
        return 1;
    }
}
#endif
