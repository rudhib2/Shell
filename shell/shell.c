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

int execute_command(char* command) {
    int status;
    pid_t new_pid = fork();

    if (new_pid == 0) {
        if (strcmp(command, "") != 0) {
            char* token = strtok(command, " ");
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
    } else if (new_pid > 0) {
        waitpid(new_pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        print_fork_failed();
        return -1;
    }

    return 0;
}

int execute_logical_command(char* logical_command) {
    char* token;
    char* command1;
    char* command2;
    int operator_type; // 0 for ;, 1 for &&, 2 for ||
    
    // Check for logical operators and split the command
    if ((token = strstr(logical_command, "&&"))) {
        operator_type = 1;
        command1 = strtok(logical_command, "&&");
        command2 = token + 2; // Move to the characters after '&&'
    } else if ((token = strstr(logical_command, "||"))) {
        operator_type = 2;
        command1 = strtok(logical_command, "||");
        command2 = token + 2; // Move to the characters after '||'
    } else {
        operator_type = 0;
        token = strstr(logical_command, ";");
        command1 = strtok(logical_command, ";");
        command2 = token + 2;
    }

    // Execute the first command
    int status1 = execute_command(command1);

    if (operator_type == 1) { // Logical AND (&&)
        // Execute the second command only if the first one succeeded
        if (status1 == 0) {
            execute_command(command2);
        }
    } else if (operator_type == 2) { // Logical OR (||)
        // Execute the second command only if the first one failed
        if (status1 != 0) {
            execute_command(command2);
        }
    } else { // Separator (;)
        // Execute the second command unconditionally
        if (command2 != NULL) {
            execute_command(command2);
        }
    }

    return status1;
}


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

        if (strstr(written_in_terminal, "&&") || strstr(written_in_terminal, "||") || strstr(written_in_terminal, ";")) {
            status = execute_logical_command(written_in_terminal);
        } else {
            status = execute_command(written_in_terminal);
        }

        if (status != 0) {
            perror("error");
            exit(1);
        }

        if (history_flag) {
            fprintf(history_file, "\n%s", written_in_terminal);
            fflush(history_file);
        }
    }
    return 0;
}