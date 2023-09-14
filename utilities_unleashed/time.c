/**
 * utilities_unleashed
 * CS 341 - Fall 2023
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include "format.h"

int main(int argc, char *argv[])
{

    pid_t pid = fork();
    struct timespec start_time, end_time;
    if (pid < 0) { 
        print_fork_failed();
    } else if (pid == 0) {
        /* child process */
        execvp(argv[1], &argv[1]);
        perror("execvp");
        exit(1);
    } else { 
        int status;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        waitpid(pid, &status, 0);
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                              (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
        display_results(argv, elapsed_time);
    }
    return 0;
}