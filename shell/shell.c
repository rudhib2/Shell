/**
 * shell
 * CS 341 - Fall 2023
 */

// I used Chat GPT for initial design and debugging code - it worked well.

#include "format.h"
#include "shell.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "vector.h"
#include <stdlib.h>
#include <sys/types.h>

#define MAX_INPUT_SIZE 100
// extern char *optarg;


typedef struct process {
    char *command;
    pid_t pid;
} process;

void execute_script(const char* filename) {
    FILE *script_file = fopen(filename, "r");
    if (script_file == NULL) {
        print_script_file_error();
        // perror("Error opening script file");
        exit(0);
    }

    char line[MAX_INPUT_SIZE];

    while (fgets(line, sizeof(line), script_file)) {
        line[strlen(line) - 1] = '\0';

        int status;
        pid_t new_pid = fork();

        if (new_pid > 0) {
            waitpid(new_pid, &status, 0);
        } else if (new_pid == 0) {
            if (strcmp(line, "") != 0) {
                char* token = strtok(line, " ");
                char* args[100];
                int i = 0;
                while (token != NULL) {
                    args[i++] = token;
                    token = strtok(NULL, " ");
                }
                args[i] = NULL;
                execvp(args[0], args);
                perror("execvp failed");
                exit(0);
            }
        } else {
            print_fork_failed();
            // perror("fork failed");
            exit(0);
        }
    }
    fclose(script_file);
}

void execute_command(const char* command) {
    // Create a copy of the command string that can be modified
    char command_copy[MAX_INPUT_SIZE];
    strcpy(command_copy, command);

    char* token = strtok(command_copy, " ");
    char* args[100];
    int i = 0;
    
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Execute the command
    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}


// Function to parse and execute commands with logical operators
void execute_logical_command(const char* command) {
    char* and_token = strstr(command, "&&");
    char* or_token = strstr(command, "||");
    char* seq_token = strstr(command, ";");

    if (and_token) {
        *and_token = '\0';
        int status;
        pid_t new_pid = fork();

        if (new_pid == 0) {
            execute_command(command);
            exit(0);
        } else if (new_pid > 0) {
            waitpid(new_pid, &status, 0);
            if (status == 0) {
                execute_logical_command(and_token + 2);
            }
        } else {
            print_fork_failed();
            exit(1);
        }
    } else if (or_token) { // Handle ||
        *or_token = '\0';
        int status;
        pid_t new_pid = fork();

        if (new_pid == 0) {
            execute_command(command);
            exit(0);
        } else if (new_pid > 0) {
            waitpid(new_pid, &status, 0);
            if (status != 0) {
                execute_logical_command(or_token + 2);
            }
        } else {
            print_fork_failed();
            exit(1);
        }
    } else if (seq_token) { // Handle ;
        *seq_token = '\0';
        execute_command(command);
        execute_logical_command(seq_token + 1);
    } else {
        execute_command(command);
    }
}



int shell(int argc, char *argv[]) {
    char curr_dir[MAX_INPUT_SIZE];
    char written_in_terminal[MAX_INPUT_SIZE];
    pid_t pid = getpid();
    
    int history_flag = 0;
    char* history_filename = NULL;
    if (argc == 3 && strcmp(argv[1], "-h") == 0) {
        history_flag = 1;
        history_filename = argv[2];
    }

    int script_flag = 0;
    char* script_filename = NULL;
    if (argc == 3 && strcmp(argv[1], "-f") == 0) {
        script_flag = 1;
        script_filename = argv[2];
    }

    FILE *history_file = NULL;
    if (history_flag) {
        history_file = fopen(history_filename, "a");
        if (history_file == NULL) {
            print_history_file_error();
            // perror("Error opening history file");
            exit(0);
        }
    }

    if (script_flag) {
        execute_script(script_filename);
        exit(0); // Exit after running the script
    }

    while(1) {
        print_prompt(getcwd(curr_dir, sizeof(curr_dir)), pid);
        fgets(written_in_terminal, sizeof(written_in_terminal), stdin);
        written_in_terminal[strlen(written_in_terminal) - 1] = '\0';

        if (strcmp(written_in_terminal, "exit") == 0) {
            if (history_flag) {
                fclose(history_file);
            }
            exit(0);
        }

        int status;
        pid_t new_pid = fork();

        if (new_pid > 0) {
            waitpid(new_pid, &status, 0);
        } else if (new_pid == 0) {
            if (strcmp(written_in_terminal, "") != 0) {
                char* token = strtok(written_in_terminal, " ");
                char* args[100];
                int i = 0;
                while (token != NULL) {
                    args[i++] = token;
                    token = strtok(NULL, " ");
                }
                args[i] = NULL;
                if (!(strstr(written_in_terminal, "&&") || strstr(written_in_terminal, "||") || strstr(written_in_terminal, ";"))) {
                    execvp(args[0], args);
                    perror("execvp failed");
                    exit(0);
                }
                
            }
        } else {
            // perror("fork failed");
            print_fork_failed();
            exit(0);
        }
        if (strstr(written_in_terminal, "&&") || strstr(written_in_terminal, "||") || strstr(written_in_terminal, ";")) {
            execute_logical_command(written_in_terminal);
        }
        // execute_logical_command(written_in_terminal);


        // Write the command to history file if the -h flag is set
        if (history_flag) {
            fprintf(history_file, "\n%s", written_in_terminal);
            fflush(history_file);
        }
    }
    return 0;
}
