/**
 * mapreduce
 * CS 341 - Fall 2023
 */
#include "utils.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    char *input;
    char *output;
    char *mapper;
    char *reducer;
    int num_mappers;

    if (argc != 6) {
        // Exit the program with an error code.
        exit(1);
    }

    input = argv[1];
    output = argv[2];
    mapper = argv[3];
    reducer = argv[4];
    num_mappers = atoi(argv[5]);

    // Create an input pipe for each mapper.
    int* input_pipe_mapper[num_mappers];
    int i = 0;
    while (i < num_mappers) {
        // Allocate memory for a pipe.
        input_pipe_mapper[i] = calloc(2, sizeof(int));
        // Create a pipe.
        pipe(input_pipe_mapper[i]);
        i++;
    }

    // Create one input pipe for the reducer.
    int input_pipe_reducer[2];
    pipe(input_pipe_reducer);

    // Open the output file.
    int output_file_open = open(output, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);

    // Start a splitter process for each mapper.
    pid_t splitter_mapper[num_mappers];
    i = 0;
    while (i < num_mappers) {
        // Create a child process.
        splitter_mapper[i] = fork();

        if (splitter_mapper[i] == -1) {
            // Exit the program with an error code.
            exit(1);
        }
        
        if (!splitter_mapper[i]) {
            // Close the read end of the pipe.
            close(input_pipe_mapper[i][0]);
            dup2(input_pipe_mapper[i][1], 1);
            char mapper_idx[10];
            // Convert the mapper index to a string.
            sprintf(mapper_idx, "%d", i);
            execl("./splitter", "./splitter", input, argv[5], mapper_idx, NULL);
            // Exit the program with an error code.
            exit(1);
        }

        i++;
    }

    // Start all the mapper processes.
    pid_t mapper_process[num_mappers];
    i = 0;
    while (i < num_mappers) {
      // Close the write end of the pipe.
      close(input_pipe_mapper[i][1]);
      // Create a child process.
      mapper_process[i] = fork();

        if (splitter_mapper[i] == -1) {
            // Exit the program with an error code.
            exit(1);
        }

        if (!mapper_process[i]) {
            // Close the read end of the reducer pipe.
            close(input_pipe_reducer[0]);
            dup2(input_pipe_mapper[i][0], 0);
            dup2(input_pipe_reducer[1], 1);
            execl(mapper, mapper, NULL);
            // Exit the program with an error code.
            exit(1);
        }

        i++;
    }

    // Start the reducer process.
    close(input_pipe_reducer[1]);
    // Create a child process.
    pid_t child = fork();

    if (child == -1) {
        // Exit the program with an error code.
        exit(1);
    }

    if (!child) {
        dup2(input_pipe_reducer[0], 0);
        dup2(output_file_open, 1);
        execl(reducer, reducer, NULL);
        // Exit the program with an error code.
        exit(1);
    }
    close(input_pipe_reducer[0]);
    close(output_file_open);

    // Wait for the reducer to finish.
    i = 0;
    while (i < num_mappers) {
        int splitter_mapper_status;
        waitpid(splitter_mapper[i], &splitter_mapper_status, 0);
        i++;
    }
    
    i = 0;
    while (i < num_mappers) {
        // Close the read end of the mapper pipe.
        close(input_pipe_mapper[i][0]);
        int mapper_process_status;
        // Wait for the mapper process to finish.
        waitpid(mapper_process[i], &mapper_process_status, 0);
        i++;
    }

    // Print nonzero subprocess exit codes.
    int child_status;
    // Wait for the reducer process to finish.
    waitpid(child, &child_status, 0);
    if (child_status) {
        print_nonzero_exit_status(reducer, child_status);
    }

    // Count the number of lines in the output file.
    print_num_lines(output);
    return 0;
}