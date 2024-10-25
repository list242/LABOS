#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#ifdef __unix__
#include <pwd.h>
#include <grp.h>
#include <getopt.h>
#include <unistd.h>
#include <dirent.h>
#endif

#define COLOR_DIR "\x1b[34m"   // Синий
#define COLOR_EXEC "\x1b[32m"  // Зеленый
#define COLOR_LINK "\x1b[35m"  // Розовый
#define COLOR_RESET "\x1b[0m"  // Сброс цвета

typedef struct {
    char name[256];
    struct stat file_stat;
} file_info_t;

void print_file_info(const char *fullpath, const char *name, struct stat *file_stat) {
    // Права доступа
    printf((S_ISDIR(file_stat->st_mode)) ? "d" : (S_ISLNK(file_stat->st_mode)) ? "l" : "-");
    printf((file_stat->st_mode & S_IRUSR) ? "r" : "-");
    printf((file_stat->st_mode & S_IWUSR) ? "w" : "-");
    printf((file_stat->st_mode & S_IXUSR) ? "x" : "-");
    printf((file_stat->st_mode & S_IRGRP) ? "r" : "-");
    printf((file_stat->st_mode & S_IWGRP) ? "w" : "-");
    printf((file_stat->st_mode & S_IXGRP) ? "x" : "-");
    printf((file_stat->st_mode & S_IROTH) ? "r" : "-");
    printf((file_stat->st_mode & S_IWOTH) ? "w" : "-");
    printf((file_stat->st_mode & S_IXOTH) ? "x" : "-");

    // Количество жестких ссылок
    printf(" %3lu", (unsigned long)file_stat->st_nlink);

    // Имя владельца и группы
    struct passwd *pw = getpwuid(file_stat->st_uid);
    struct group *gr = getgrgid(file_stat->st_gid);
    printf(" %-8s %-8s", pw ? pw->pw_name : "", gr ? gr->gr_name : "");

    // Размер файла
    printf(" %8lld", (long long)file_stat->st_size);

    // Время последнего изменения
    char timebuf[80];
    struct tm *tm_info = localtime(&file_stat->st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);
    printf(" %s ", timebuf);

    // Обработка цветного вывода и символических ссылок
    if (S_ISLNK(file_stat->st_mode)) {
        char link_target[1024];
        ssize_t len = readlink(fullpath, link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            printf("%s -> %s\n", name, link_target);
            printf(COLOR_LINK "%-20s" COLOR_RESET " -> %s\n", name, link_target);
        } else {
            printf(COLOR_LINK "%-20s" COLOR_RESET "\n", name);
        }
    } else if (S_ISDIR(file_stat->st_mode)) {
        printf(COLOR_DIR "%-20s" COLOR_RESET "\n", name);
    } else if (file_stat->st_mode & S_IXUSR) {
        printf(COLOR_EXEC "%-20s" COLOR_RESET "\n", name);
    } else {
        printf("%-20s\n", name);
    }
}

// Функция сортировки файлов
int compare_files(const void *a, const void *b) {
    const file_info_t *fileA = (const file_info_t *)a;
    const file_info_t *fileB = (const file_info_t *)b;

    if (fileA->name[0] == '.' && fileB->name[0] != '.') {
        return -1;
    }
    if (fileA->name[0] != '.' && fileB->name[0] == '.') {
        return 1;
    }

    if (isupper(fileA->name[0]) && islower(fileB->name[0])) {
        return -1;
    }
    if (islower(fileA->name[0]) && isupper(fileB->name[0])) {
        return 1;
    }

    return strcasecmp(fileA->name, fileB->name);
}

void list_files(const char *directory, int show_all, int long_list) {
#ifdef __unix__
    DIR *dp = opendir(directory);
    if (!dp) {
        perror("opendir");
        return;
    }

    file_info_t *files = NULL;
    size_t count = 0;
    long long total_blocks = 0;
    struct dirent *entry;

    // Сбор информации о файлах
    while ((entry = readdir(dp)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        }

        files = realloc(files, (count + 1) * sizeof(file_info_t));
        strncpy(files[count].name, entry->d_name, sizeof(files[count].name) - 1);
        files[count].name[sizeof(files[count].name) - 1] = '\0';

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", directory, entry->d_name);

        if (lstat(fullpath, &files[count].file_stat) == -1) {
            perror("lstat");
            free(files);
            closedir(dp);
            return;
        }

        total_blocks += files[count].file_stat.st_blocks;
        count++;
    }

    closedir(dp);

    // Вывод общего количества блоков
    if (long_list) {
        printf("total %lld\n", total_blocks / 2);
    }

    qsort(files, count, sizeof(file_info_t), compare_files);

    // Вывод информации о каждом файле
    for (size_t i = 0; i < count; i++) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", directory, files[i].name);
        if (long_list) {
            print_file_info(fullpath, files[i].name, &files[i].file_stat);
        } else {
            if (S_ISDIR(files[i].file_stat.st_mode)) {
                printf(COLOR_DIR "%s" COLOR_RESET "  ", files[i].name);
            } else if (files[i].file_stat.st_mode & S_IXUSR) {
                printf(COLOR_EXEC "%s" COLOR_RESET "  ", files[i].name);
            } else if (S_ISLNK(files[i].file_stat.st_mode)) {
                printf(COLOR_LINK "%s" COLOR_RESET "  ", files[i].name);
            } else {
                printf("%s  ", files[i].name);
            }
        }
    }

    if (!long_list) {
        printf("\n");
    }

    free(files);
#else
    fprintf(stderr, "This program is only supported on UNIX systems.\n");
#endif
}
//
int main(int argc, char *argv[]) {
    int show_all = 0, long_list = 0, opt;
    while ((opt = getopt(argc, argv, "la")) != -1) {
        switch (opt) {
            case 'l':
                long_list = 1;
                break;
            case 'a':
                show_all = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-a] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    const char *directory = (optind < argc) ? argv[optind] : ".";
    list_files(directory, show_all, long_list);

    return 0;
}
