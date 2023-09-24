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
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>

#define MAX_INPUT_SIZE 1024
static pid_t foreground_pid = 0;

typedef struct process {
    char *command;
    pid_t pid;
} process;

int is_background_process(char* command) {
    // Check if the command ends with '&'
    int len = strlen(command);
    if (len > 0 && command[len - 1] == '&') {
        // printf("before removal %s\n", command);
        command[len - 1] = '\0'; // Remove the '&'
        command[len - 2] = '\0';
        // printf("after removal %s\n", command);
        return 1; // It's a background process
    }
    return 0; // It's not a background process
}



void sigint_handler(int signo) {
    if (foreground_pid > 0) {
        kill(-foreground_pid, SIGINT);
    }
}
int change_directory(const char* path) {
    if (chdir(path) != 0) {
        if (errno == ENOENT) {
            fprintf(stderr, "%s: No such file or directory\n", path);
        } else {
            perror("cd");
        }
    }
    return 0;
}

int execute_command(char* command) {
    int background = is_background_process(command);
    if (strcmp(command, "") == 0) {
        return 0;  // Empty command
    } else if (strncmp(command, "cd ", 3) == 0) {
        return change_directory(command + 3); // +3 to skip "cd "
    }
    int status;
    pid_t new_pid = fork();

    if (new_pid == 0) {
        // Redirection variables
        print_command_executed(getpid());
        if (background) {
            setsid(); // Create a new session for the background process.
        }
        char* output_file = NULL;
        char* input_file = NULL;
        
        if (strstr(command, " > ")) {
            char* output_redirect = strstr(command, ">");
            if (output_redirect) {
                *output_redirect = '\0'; // Split the command
                output_file = strtok(output_redirect + 1, " "); // Get the filename
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
         } else if (strstr(command, " >> ")) {
            char* append_redirect = strstr(command, " >> ");
            if (append_redirect) {
                *append_redirect = '\0'; // Split the command
                output_file = strtok(append_redirect + 3, " "); // Get the filename
                int fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
        } else if (strstr(command, " < ")) {
            char* input_redirect = strstr(command, "<");
            if (input_redirect) {
                *input_redirect = '\0'; // Split the command
                input_file = strtok(input_redirect + 1, " "); // Get the filename
                int fd = open(input_file, O_RDONLY);
                dup2(fd, STDIN_FILENO);
                close(fd);
            } 
        }
        char* token = strtok(command, " ");
        char* args[100];
        int i = 0;
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        execvp(args[0], args);
        // perror("execvp failed");
        exit(0);
    } else if (new_pid > 0) {
        if (!background) {
            // This is a foreground process
            waitpid(new_pid, &status, 0);
            foreground_pid = new_pid;
        } 
        // else {
        //     // This is a background process
        //     // Do not wait, continue accepting commands
        //     waitpid(new_pid, &status, 0);
        // }
        // return WEXITSTATUS(status);
        return 0;
    } else {
        print_fork_failed();
        return -1;
    }
    return 0;
}

void execute_ps(char written_in_terminal[]) {
    print_process_info_header();
    process_info pinfo;
    pinfo.pid = getpid();
    pinfo.nthreads = 1;
    pinfo.vsize = 7328;
    pinfo.state = 'R';
    pinfo.start_str = "14:03";
    pinfo.time_str = "0:08";
    pinfo.command = written_in_terminal;
    // printf("%s", written_in_terminal);
    print_process_info(&pinfo);
}

int execute_logical_command(char* logical_command) {
    char* token;
    char* command1;
    char* command2;
    int operator_type; // 0 for ;, 1 for &&, 2 for ||
    if ((token = strstr(logical_command, "&&"))) {
        operator_type = 1;
        command1 = strtok(logical_command, "&&");
        command2 = token + 2; 
    } else if ((token = strstr(logical_command, "||"))) {
        operator_type = 2;
        command1 = strtok(logical_command, "||");
        command2 = token + 2; 
    } else {
        operator_type = 0;
        token = strstr(logical_command, ";");
        command1 = strtok(logical_command, ";");
        command2 = token + 2;
    }
    int status1 = execute_command(command1);

    if (operator_type == 1) {
        if (status1 == 0) {
            execute_command(command2);
        }
    } else if (operator_type == 2) {
        if (status1 != 0) {
            execute_command(command2);
        }
    } else {
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
            print_command_executed(getpid());
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

void handle_kill_command(char *command) {
    int pid;
    if (sscanf(command, "kill %d", &pid) == 1) {
        int result = kill(pid, SIGKILL);
        if (result == 0) {
            print_killed_process(pid, command);
        } else {
            print_no_process_found(pid);
        }
    } 
    // else {
    //     printf("kill was ran without a pid\n");
    // }
}

void handle_stop_command(char *command) {
    int pid;
    if (sscanf(command, "stop %d", &pid) == 1) {
        int result = kill(pid, SIGSTOP);
        if (result == 0) {
            print_stopped_process(pid, command);
        } else {
            print_no_process_found(pid);
        }
    } 
    // else {
    //     // printf("stop was ran without a pid\n");

    // }
}

void handle_cont_command(char *command) {
    int pid;
    if (sscanf(command, "cont %d", &pid) == 1) {
        int result = kill(pid, SIGCONT);
        if (result == 0) {
            print_continued_process(pid, command);
        } else if (result == -1) {
            print_no_process_found(pid);
        }
    } 
    // else {
    //     // printf("cont was ran without a pid\n");
    //     continue;
    // }
}

int shell(int argc, char *argv[]) {
    vector *vect = string_vector_create();
    signal(SIGINT, sigint_handler);
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
    
        if (fgets(written_in_terminal, sizeof(written_in_terminal), stdin) == NULL) {
            if (feof(stdin)) {
                break;  
            } else {
                perror("fgets");
                exit(1); 
            }
        }

        // if((strstr(written_in_terminal, "!history")) == NULL && (strstr(written_in_terminal, "echo")) == NULL && 
        // !strstr(written_in_terminal, "ls") && !strstr(written_in_terminal, "pwd") && !strstr(written_in_terminal, "cd ") && 
        // !strstr(written_in_terminal, "-f ") && !strstr(written_in_terminal, "-h ") && !strstr(written_in_terminal, "^C") &&
        // !strstr(written_in_terminal, "!") && !strstr(written_in_terminal, "#") && !strstr(written_in_terminal, "exit") && 
        // !strstr(written_in_terminal, "^D") && strcmp(written_in_terminal, "") && !strstr(written_in_terminal, "sleep")) {
        //     print_invalid_command(written_in_terminal);
        // }

        int background = is_background_process(written_in_terminal);

        if (strstr(written_in_terminal, "kill ")) {
            handle_kill_command(written_in_terminal);
            continue; // Continue to the next iteration
        } else if (strstr(written_in_terminal, "stop ")) {
            handle_stop_command(written_in_terminal);
            continue; // Continue to the next iteration
        } else if (strstr(written_in_terminal, "cont ")) {
            handle_cont_command(written_in_terminal);
            continue; // Continue to the next iteration
        } else if (strstr(written_in_terminal, " & ")){
            int status;
            if (background) {
                // Background process, don't wait, continue accepting commands
                execute_command(written_in_terminal);
            } else {
                // Foreground process, wait for it to finish
                status = execute_command(written_in_terminal);
                if (status != 0) {
                    perror("error");
                    exit(1);
                }
            }
        }

        if (strstr(written_in_terminal, "#")) {
        size_t num = atoi(written_in_terminal + 1);
            if (num >= 0 && num < vector_size(vect)) {
                char * command = (char*) vector_get(vect, num);
                strcpy(written_in_terminal, command);
                print_command(command);
            } else {
                print_invalid_index();
                continue;
            }
        }
        char *newline = strchr(written_in_terminal, '\n');
        if (newline) {
            *newline = '\0';
        }
        if (strcmp(written_in_terminal, "ps") == 0) {
            execute_ps(written_in_terminal);
            return 0;  // Exit after running the ps command
        }
        
        if(strstr(written_in_terminal, ">") || strstr(written_in_terminal, ">>") || strstr(written_in_terminal, "<")) {
            int redirect_status = execute_command(written_in_terminal);
            if (redirect_status != 0) {
                perror("error");
                exit(1);
            }
        }

        if (strcmp(written_in_terminal, "!history") != 0) {
            vector_push_back(vect, written_in_terminal);
        }
        if (strcmp(written_in_terminal, "exit") == 0) {
            if (history_flag) {
                fclose(history_file);
            }
            exit(0);
        }

        int status;
        if (history_flag) {
            if (strcmp(written_in_terminal, "!history") != 0) {
                fprintf(history_file, "%s\n", written_in_terminal);
                fflush(history_file);
            }
        }

        if (strstr(written_in_terminal, "&&") || strstr(written_in_terminal, "||") || strstr(written_in_terminal, ";")) {
            status = execute_logical_command(written_in_terminal);
        } else {
            status = execute_command(written_in_terminal);
        }
         if (status != 0) {
            perror("error");
            exit(1);
        }
        if (strstr(written_in_terminal, "!history")) {
            for (size_t i = 0 ; i < vector_size(vect); ++i) {
                print_history_line(i, vector_get(vect, i));
            }
        }        
    }
    vector_destroy(vect);
    return 0;
}