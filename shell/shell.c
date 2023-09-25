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
static vector *process_l;

int is_background_process(char* command) {
    int len = strlen(command);
    if (len > 0 && command[len - 1] == '&') {
        command[len - 1] = '\0'; // Remove the '&'
        command[len - 2] = '\0';
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
    if (strcmp(command, "!history") == 0) {
        return 0;  
    } 
    if (command[0] == '!') {
        return 0;  
    } 
    int background = is_background_process(command);
    if (strcmp(command, "") == 0) {
        return 0;  
    } else if (strncmp(command, "cd ", 3) == 0) {
        return change_directory(command + 3); // +3 to skip "cd "
    }
    int status;
    pid_t new_pid = fork();
    if (new_pid == 0) {
        // Redirection variables
        print_command_executed(getpid());
        if (background) {
            setsid();
        }
        char* output_file = NULL;
        char* input_file = NULL;
        
        if (strstr(command, " > ")) {
            char* output_redirect = strstr(command, ">");
            if (output_redirect) {
                *output_redirect = '\0'; 
                output_file = strtok(output_redirect + 1, " "); 
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
         } else if (strstr(command, " >> ")) {
            char* append_redirect = strstr(command, " >> ");
            if (append_redirect) {
                *append_redirect = '\0'; 
                output_file = strtok(append_redirect + 3, " "); 
                int fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
        } else if (strstr(command, " < ")) {
            char* input_redirect = strstr(command, "<");
            if (input_redirect) {
                *input_redirect = '\0'; 
                input_file = strtok(input_redirect + 1, " "); 
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
        print_exec_failed(args[0]);
        exit(1);
    } else if (new_pid > 0) {
        if (!background) {
            waitpid(new_pid, &status, 0);
            foreground_pid = new_pid;
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                fprintf(stderr, "Command '%s' exited with status %d\n", command, WEXITSTATUS(status));
            }
        } 
        // waitpid(new_pid, &status, 0);
        // return WEXITSTATUS(status);        
        // return 0;
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
                print_exec_failed(args[0]);
                exit(1);
            }
        } else {
            print_fork_failed();
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
}

int shell(int argc, char *argv[]) {
    // vector *bg_vect = string_vector_create();
    process_l = shallow_vector_create();
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
            exit(0);
        }
    }
    if (script_flag) {
        execute_script(script_filename);
        exit(0); 
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
                // vector_push_back(vector_l, written_in_terminal);
                execute_command(written_in_terminal);
            } else {
                status = execute_command(written_in_terminal);
                if (status != 0) {
                    perror("error");
                    exit(1);
                }
            }
        } else if (written_in_terminal[0] == '!' && (strcmp(written_in_terminal, "!history") != 0)) {
            char* prefix;
            prefix = written_in_terminal + 1;
            prefix[strlen(prefix)] = '\0';
            for (size_t i = vector_size(vect) - 1 ; i >= 0; --i) {
                char* command = strdup(vector_get(vect,i));
                if (strncmp(command, prefix, strlen(prefix)-1) == 0) {
                    print_command(command);
                    int status = execute_command(command);
                    if (status != 0) {
                        perror("error");
                        exit(1);
                    }
                    break;
                }
                if(i == 0) {
                    print_no_history_match();
                    // vector_pop_back(vect);
                    break;
                }
            }
        }
        if (strstr(written_in_terminal, "#")) {
        size_t num = atoi(written_in_terminal + 1);
            if (num < 0 || num >= vector_size(vect)) {
                print_invalid_index();
                // return 0;
                continue;
            } 
            char * command = (char*) vector_get(vect, num);
            strcpy(written_in_terminal, command);
            print_command(command);
        }
        char *newline = strchr(written_in_terminal, '\n');
        if (newline) {
            *newline = '\0';
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