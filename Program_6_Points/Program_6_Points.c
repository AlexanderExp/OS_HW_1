#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 8192

int main(int argc, char *argv[]) {
    int pipefd[2], pipefd1[2];
    int pid;
    char buffer[BUFFER_SIZE] = {0};
    int n, num_digits = 0, num_letters = 0;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipefd1) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {    // Второй процесс
        n = close(pipefd[1]); /* Close unused write end of the pipe */
        if (n < 0) {
            perror("Wrong");
            exit(1);
        }

        /* Read input from parent process */
        n = read(pipefd[0], buffer, BUFFER_SIZE);

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

        n = close(pipefd[0]);
        if (n < 0) {
            perror("Wrong");
            exit(1);
        }

        snprintf(buffer, BUFFER_SIZE, "Digits: %d, Letters: %d\n", num_digits, num_letters);
        n = write(pipefd1[1], buffer, BUFFER_SIZE);

        /* Write results to parent process */
        if (n < 0) {
            perror("Write");
            exit(1);
        }

        n = close(pipefd1[1]);
        if (n < 0) {
            perror("Wrong");
            exit(1);
        }
        exit(EXIT_SUCCESS);
    } else {              /* Parent process */
        close(pipefd[0]); /* Close unused read end of the pipe */

        /* Read input from file */
        int input_file = open(argv[1], O_RDONLY, 0666);

        if (input_file < 0) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        n = read(input_file, buffer, BUFFER_SIZE);

        close(input_file);
        if (n < 0) {
            perror("fread");
            exit(EXIT_FAILURE);
        }

        /* Write input to child process */
        if (write(pipefd[1], buffer, BUFFER_SIZE) == -1) {
            perror("writesdf");
            exit(EXIT_FAILURE);
        }

        n = close(pipefd[1]); /* Close write end of the pipe */
        if (n < 0) {
            perror("Wrong");
            exit(1);
        }
        // waitpid(pid, NULL, 0);
    }
    while (wait(NULL) > 0)
        ;
    close(pipefd1[1]);

    if (read(pipefd1[0], buffer, BUFFER_SIZE) == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    close(pipefd1[0]);

    int output_file = open(argv[2], O_WRONLY | O_CREAT, 0666);
    if (output_file < 0) {
        perror("Wrong end");
        exit(1);
    }

    n = write(output_file, buffer, strlen(buffer));

    if (n < 0) {
        perror("Wrong very end");
        exit(1);
    }

    close(output_file);
    return 0;
}