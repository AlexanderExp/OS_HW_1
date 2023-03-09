#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUF_SIZE 8192

int main(int argc, char *argv[]) {
    // Проверка наличия аргументов командной строки
    if (argc != 3) {
        printf("Usage: %s input_file output_file\n", argv[0]);
        exit(1);
    }

    // Открытие файла для чтения
    int input_fd = open(argv[1], O_RDONLY);
    if (input_fd == -1) {
        perror("open");
        exit(1);
    }

    // Создание неименованного канала
    int channel1[2];
    if (pipe(channel1) == -1) {
        perror("pipe");
        exit(1);
    }

    // Создание второго процесса
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        exit(1);
    }

    if (pid2 == 0) {// Второй процесс
        // Закрытие неиспользуемого конца канала
        close(channel1[1]);

        // Чтение данных из канала
        char buf[MAX_BUF_SIZE];
        ssize_t n = read(channel1[0], buf, MAX_BUF_SIZE);
        if (n == -1) {
            perror("read");
            exit(1);
        }

        // Вычисление количества цифр и букв
        ssize_t i;
        int digits = 0, letters = 0;
        for (i = 0; i < n; i++) {
            if (isdigit(buf[i])) {
                digits++;
            } else if (isalpha(buf[i])) {
                letters++;
            }
        }

        // Создание неименованного канала
        int channel2[2];
        if (pipe(channel2) == -1) {
            perror("pipe");
            exit(1);
        }

        // Создание третьего процесса
        pid_t pid3 = fork();
        if (pid3 == -1) {
            perror("fork");
            exit(1);
        }

        if (pid3 == 0) {// Третий процесс
            // Закрытие неиспользуемого конца канала
            close(channel2[1]);

            // Чтение данных из канала
            int result;
            n = read(channel2[0], &result, sizeof(result));
            if (n == -1) {
                perror("read");
                exit(1);
            }

            // Открытие файла для записи
            int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                perror("open");
                exit(1);
            }

            // Вывод результата в файл
            char buffer[MAX_BUF_SIZE];
            snprintf(buffer, MAX_BUF_SIZE, "Digits: %d, Letters: %d\n", digits, letters);
            n = write(output_fd, buffer, strlen(buffer));
            if (n == -1) {
                perror("write");
                exit(1);
            }
            close(output_fd);

            // Закрытие канала
            // close(channel2[0]);

            // Выход из процесса
            exit(0);
        } else {
            close(channel2[0]);
            close(channel1[0]);

            // Запись результата в канал
            int result = digits + letters;
            n = write(channel2[1], &result, sizeof(result));
            if (n == -1) {
                perror("write");
                exit(1);
            }

            // Закрытие канала
            close(channel2[1]);
            // Ожидание завершения третьего процесса
            wait(NULL);

            // Закрытие канала
            close(channel1[0]);

            // Выход из процесса
            exit(0);
        }
    } else {// Первый процесс
        // Закрытие неиспользуемого конца канала
        close(channel1[0]);

        // Чтение данных из файла
        char buf[MAX_BUF_SIZE];
        int n = read(input_fd, buf, MAX_BUF_SIZE);
        if (n == -1) {
            perror("read");
            exit(1);
        }

        // Запись данных в канал
        n = write(channel1[1], buf, n);
        if (n == -1) {
            perror("write");
            exit(1);
        }

        // Закрытие конца канала
        close(channel1[1]);

        // Ожидание завершения второго процесса
        waitpid(pid2, NULL, 0);

        // Завершение первого процесса
        exit(0);
    }
}
