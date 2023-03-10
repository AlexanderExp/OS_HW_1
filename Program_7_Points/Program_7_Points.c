#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

int main(int argc, char *argv[]) {
    int fd, fd1;
    int pid;
    char buffer[BUFFER_SIZE] = {0};
    int n, num_digits = 0, num_letters = 0;
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    mkfifo("fifo1", 0666);// Создаем первый именованный канал
    mkfifo("fifo2", 0666);// Создаем второй именованный канал

    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {           // Второй процесс
        fd = open("fifo1", O_RDONLY);// Открываем первый канал на чтение
        if (fd < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        fd1 = open("fifo2", O_WRONLY);// Открываем второй канал на запись
        if (fd1 < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        /* Read input from parent process */
        n = read(fd, buffer, BUFFER_SIZE);

        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* Count the number of digits and letters in the input string */
        for (int i = 0; i < n; i++) {
            if (isdigit(buffer[i])) {
                num_digits++;
            } else if (isalpha(buffer[i])) {
                num_letters++;
            }
        }

        snprintf(buffer, BUFFER_SIZE, "Digits: %d, Letters: %d\n", num_digits, num_letters);
        n = write(fd1, buffer, BUFFER_SIZE);

        /* Write results to parent process */
        if (n < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        close(fd);
        close(fd1);
        exit(EXIT_SUCCESS);
    } else {                         /* Parent process */
        fd = open("fifo1", O_WRONLY);// Открываем первый канал на запись
        if (fd < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        /* Read input from file */
        int input_file = open(argv[1], O_RDONLY, 0666);

        if (input_file < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        n = read(input_file, buffer, BUFFER_SIZE);

        close(input_file);
        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* Write input to child process */
        if (n == write(fd, buffer, BUFFER_SIZE) < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        fd1 = open("fifo2", O_RDONLY);// Открываем второй канал на чтение
        if (fd1 < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        /* Read results from child process */
        n = read(fd1, buffer, BUFFER_SIZE);

        /* Write results to output file */
        int output_file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);

        if (output_file < 0) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        n = write(output_file, buffer, strlen(buffer));

        if (n < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        close(fd);
        close(fd1);
        close(output_file);
        wait(NULL);// Ожидаем завершения дочернего процесса
        unlink("fifo1");
        unlink("fifo2");
        exit(EXIT_SUCCESS);
    }
}