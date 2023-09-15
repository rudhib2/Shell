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


typedef struct process {
    char *command;
    pid_t pid;
} process;


// int shell(int argc, char *argv[]) {
//     char curr_dir[MAX_INPUT_SIZE];
//     char written_in_terminal[MAX_INPUT_SIZE];
//     pid_t pid = getpid();
    
//     int history_flag = 0;
//     char* history_filename = NULL;
//     if (argc == 3 && strcmp(argv[1], "-h") == 0) {
//         history_flag = 1;
//         history_filename = argv[2];
//     }

//     int file_flag = 0;
//     char* file_filename = NULL;
//     if (argc == 3 && strcmp(argv[1], "-f") == 0) {
//         file_flag = 1;
//         file_filename = argv[2];
//     }

//     //TO-DO: read from file_filename 
//     //execute the commands in the file 
    
//     FILE *history_file = NULL;
//     if (history_flag) {
//         history_file = fopen(history_filename, "a");
//         if (history_file == NULL) {
//             perror("Error opening history file");
//             exit(0);
//         }
//     } 
//     while(1) {
//         print_prompt(getcwd(curr_dir, sizeof(curr_dir)), pid);
//         fgets(written_in_terminal, sizeof(written_in_terminal), stdin);
//         written_in_terminal[strlen(written_in_terminal) - 1] = '\0';
//         if (strcmp(written_in_terminal, "exit") == 0) {
//             if (history_flag) {
//                 fclose(history_file);
//             }
//             exit(0);
//         }
//         int status;
//         pid_t new_pid = fork();
//         if (new_pid > 0) {
//             waitpid(new_pid, &status, 0);
//         } else if (new_pid == 0) {
//             if (strcmp(written_in_terminal, "") != 0) {
//                 char* token = strtok(written_in_terminal, " ");
//                 char* args[100];
//                 int i = 0;
//                 while (token != NULL) {
//                     args[i++] = token;
//                     token = strtok(NULL, " ");
//                 }
//                 args[i] = NULL; 
//                 print_command_executed(getpid());
//                 execvp(args[0], args);
//                 perror("execvp failed");
//                 exit(1);
//             }
//         } else {
//             perror("fork failed");
//             exit(1);
//         }
//         // Write the command to history file if the -h flag is set
//         if (history_flag) {
//             fprintf(history_file, "\n%s", written_in_terminal);
//             fflush(history_file);
//         }
//     }
//     return 0;
// }


void execute_script(const char* filename) {
    FILE *script_file = fopen(filename, "r");
    if (script_file == NULL) {
        perror("Error opening script file");
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
            perror("fork failed");
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
    int script_flag = 0;
    char* script_filename = NULL;

    int opt = 0;
    while ((opt = getopt(argc, argv, "hf:")) != -1) {
        switch (opt) {
            case 'h':
                history_flag = 1;
                history_filename = optarg;
                break;
            case 'f':
                script_flag = 1;
                script_filename = optarg;
                break;
            default:
                perror("Invalid arguments");
                exit(0);
        }
    }

    FILE *history_file = NULL;
    if (history_flag) {
        history_file = fopen(history_filename, "a");
        if (history_file == NULL) {
            perror("Error opening history file");
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
                execvp(args[0], args);
                perror("execvp failed");
                exit(1);
            }
        } else {
            perror("fork failed");
            exit(1);
        }

        // Write the command to history file if the -h flag is set
        if (history_flag) {
            fprintf(history_file, "\n%s", written_in_terminal);
            fflush(history_file);
        }
    }
    return 0;
}
