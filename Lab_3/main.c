#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

pid_t child_pid = 0;  // Глобальная переменная для отслеживания дочернего процесса

void cleanup() {
    if (child_pid > 0) {
        printf("Завершение дочернего процесса (PID: %d).\n", child_pid);
        kill(child_pid, SIGTERM); // Завершаем дочерний процесс, если он работает
    }
    printf("Основная программа завершена.\n");
}

void handle_sigint(int signum) {
    printf("Получен сигнал SIGINT (номер: %d). Прерывание программы.\n", signum);
    cleanup(); // Выполняем очистку и завершаем программу
    exit(EXIT_SUCCESS);
}

void handle_sigterm(int signum) {
    printf("Получен сигнал SIGTERM (номер: %d). Завершение программы.\n", signum);
    cleanup(); // Выполняем очистку и завершаем программу
    exit(EXIT_SUCCESS);
}

void child_process() {
    printf("Дочерний процесс (PID: %d) запущен. Выполняется задача...\n", getpid());
    sleep(5);  // Имитация работы
    printf("Дочерний процесс завершает работу.\n");
    exit(EXIT_SUCCESS);
}

int main() {
    if (atexit(cleanup) != 0) {
        perror("Ошибка при регистрации обработчика выхода");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("Не удалось установить обработчик сигнала SIGINT");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = handle_sigterm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Не удалось установить обработчик сигнала SIGTERM");
        exit(EXIT_FAILURE);
    }

    child_pid = fork();

    if (child_pid < 0) {
        perror("Ошибка при вызове fork");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {
        // Дочерний процесс
        child_process();
    } else {
        // Родительский процесс
        printf("Родительский процесс (PID: %d), дочерний процесс PID: %d\n", getpid(), child_pid);

        int status;
        pid_t wpid = waitpid(child_pid, &status, 0);
        if (wpid == -1) {
            perror("Ошибка при ожидании дочернего процесса");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            printf("Дочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Дочерний процесс был завершен сигналом: %d\n", WTERMSIG(status));
        }
    }

    return 0;
}
