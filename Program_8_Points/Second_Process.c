#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

int main() {
    int fd, fd1;
    char buffer[BUFFER_SIZE] = {0};
    int n, num_digits = 0, num_letters = 0;

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

    // Считываем из первого процесса
    n = read(fd, buffer, BUFFER_SIZE);

    if (n < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // Считаем количество цифр и букв
    for (int i = 0; i < n; i++) {
        if (isdigit(buffer[i])) {
            num_digits++;
        } else if (isalpha(buffer[i])) {
            num_letters++;
        }
    }

    snprintf(buffer, BUFFER_SIZE, "Digits: %d, Letters: %d\n", num_digits, num_letters);
    n = write(fd1, buffer, BUFFER_SIZE);

    if (n < 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    close(fd);
    close(fd1);
    return 0;
}