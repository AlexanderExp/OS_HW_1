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
    char buffer[BUFFER_SIZE] = {0};
    int n;

    // umask(0);
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    mkfifo("fifo1", 0666);// Создаем первый именованный канал
    mkfifo("fifo2", 0666);// Создаем второй именованный канал

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

    // Записываем в первый канал
    if (n == write(fd, buffer, BUFFER_SIZE) < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    fd1 = open("fifo2", O_RDONLY);// Открываем второй канал на чтение
    if (fd1 < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Читаем из второго канала
    n = read(fd1, buffer, BUFFER_SIZE);
    if (n < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }


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
    return 0;
}