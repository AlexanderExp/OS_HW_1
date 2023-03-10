#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FIFO_NAME1 "file10.fifo"
#define FIFO_NAME2 "file20.fifo"
#define MAX_BUF_SIZE 8192

int main(int argc, char *argv[]) {
    // Проверка аргументов командной строки
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        exit(1);
    }

    mknod(FIFO_NAME1, S_IFIFO | 0666, 0);
    mknod(FIFO_NAME2, S_IFIFO | 0666, 0);
    int channel1, channel2 = 0;

    char buf[MAX_BUF_SIZE] = {0};

    // Создание дочерних процессов
    int pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(1);
    } else if (pid1 > 0) {// Первый процесс
        // Открытие файла
        int input_file = open(argv[1], O_RDONLY, 0666);
        if (input_file < 0) {
            printf("Can't open input file");
            exit(1);
        }

        // Считывание данных из файла и запись в канал
        channel1 = open(FIFO_NAME1, O_WRONLY);
        if (channel1 == -1) {
            perror("can't open channel 1 for writing");
            exit(1);
        }


        int n = read(input_file, buf, MAX_BUF_SIZE);
        if (n < 0) {
            perror("Incorrect behaviour in reading input file");
            exit(1);
        }
        close(input_file);

        n = write(channel1, buf, MAX_BUF_SIZE);
        if (n != MAX_BUF_SIZE) {
            printf("Can't write everything");
            exit(1);
        }
        close(channel1);
    } else {// Второй процесс
        // Создание второго процесса
        waitpid(pid1, NULL, 0);
        int pid2 = fork();
        if (pid2 == -1) {
            perror("fork");
            exit(1);
        } else if (pid2 > 0) {// Второй процесс
            // Открытие именованных каналов
            channel1 = open(FIFO_NAME1, O_RDONLY);
            if (channel1 == -1) {
                perror("open");
                exit(1);
            }

            // Чтение данных из канала
            int n = read(channel1, buf, MAX_BUF_SIZE);
            if (n == -1) {
                perror("read");
                exit(1);
            }

            // Вычисление количества букв и цифр
            int letters = 0;
            int digits = 0;
            for (int i = 0; i < n; i++) {
                if (isalpha(buf[i])) {
                    letters++;
                } else if (isdigit(buf[i])) {
                    digits++;
                }
            }

            // Закрытие файловых дескрипторов
            n = close(channel1);
            if (n == -1) {
                perror("can't close channel 1 in second process");
                exit(1);
            }

            channel2 = open(FIFO_NAME2, O_WRONLY);
            if (channel2 == -1) {
                perror("open");
                exit(1);
            }
            printf("Digits: %d, Letters: %d\n", digits, letters);
            snprintf(buf, MAX_BUF_SIZE, "Digits: %d, Letters: %d\n", digits, letters);

            // Запись результата в канал
            n = write(channel2, &buf, sizeof(buf));
            if (n == -1) {
                perror("write");
                exit(1);
            }
            n = close(channel2);
            if (n == -1) {
                perror("can't close channel 2 in second process");
                exit(1);
            }
        } else {
            waitpid(pid2, NULL, 0);
            channel2 = open(FIFO_NAME2, O_RDONLY);
            if (channel2 == -1) {
                perror("can't open channel 2 in third process");
                exit(1);
            }
            int n = read(channel2, buf, MAX_BUF_SIZE);
            if (n == -1) {
                perror("can't read from channel 2 in third process");
                exit(1);
            }
            n = close(channel2);
            if (n == -1) {
                perror("can't close channel 2 in third process");
                exit(1);
            }
            int output_fd = open(argv[2], O_WRONLY | O_CREAT, 0666);
            if (output_fd == -1) {
                perror("open");
                exit(1);
            }
            n = write(output_fd, buf, strlen(buf));
            if (n == -1) {
                perror("can't write in third process");
                exit(1);
            }
            close(output_fd);
            unlink(FIFO_NAME2);
            unlink(FIFO_NAME1);
        }
    }
    return 0;
}