/**
 * utilities_unleashed
 * CS 341 - Fall 2023
 */

#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "format.h"
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_time_usage();
    }
    struct timespec start_time;
    struct timespec end_time;
    double time_elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    pid_t pid = fork();
    if (pid > 0) {
        int child_status;
        waitpid(pid, &child_status, 0);
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        if (!WEXITSTATUS(child_status) && WIFEXITED(child_status)) {
            time_elapsed = (end_time.tv_sec - start_time.tv_sec) + (double) (end_time.tv_sec - start_time.tv_sec) / (double) 1e9;
            display_results(argv, time_elapsed);
        }
    } else if (pid < 0) {
        print_fork_failed();
    } else {
        execvp(argv[1], argv + 1);
        print_exec_failed();
    }
    return 0;
}