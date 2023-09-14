#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include "format.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        print_env_usage();
    }

    pid_t pid = fork();

    if (pid > 0) {
        int child_status;
        waitpid(pid, &child_status, 0);
    } else if (pid < 0) {
        print_fork_failed();
    } else {
        for(int i = 1; argv[i] != NULL; i++) {
            if (strcmp(argv[i], "--") != 0) {
                char *key = strtok(argv[i], "=");
                char *value = strtok(NULL, "");
                if (value == NULL) {
                    print_env_usage();
                } 
            
                char *iter = key;
                while(*iter) {
                    if (!(*iter == '_' || isalpha(*iter) || isdigit(*iter))) {
                        print_env_usage();
                    }
                    iter++;
                }
                if (value[0] == '%') {
                    value = getenv(value+1);
                    if (!value) {
                        print_environment_change_failed();
                    }
                } else {
                    char *iter = value;
                    while (*iter) {
                        if (!(*iter == '_' || isalpha(*iter) || isdigit(*iter))) {
                        print_env_usage();
                        }
                        iter++;
                    }
                }
                if (setenv(key,value, 1) < 0) {
                    print_environment_change_failed();
                }

            } else {
                execvp(argv[i+1], argv+i+1);
                print_exec_failed();
            }
                
            }
            print_env_usage();

        }
        return 0;
    }