//used chatGPT for debugging, initial structure, and for generating functions

/**
 * nonstop_networking
 * CS 341 - Fall 2023
 */
#include "format.h"
#include "common.h"
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>

char **parse_args(int argc, char **argv);
verb check_args(char **my_args);
int printing_errors(size_t read_bytes, size_t size);
int connect_to_server(char* host, char* port);
int execute_request(verb request);
int write_client_request(verb request);
int handle_put();
static char** my_args;
static int sock_fd;
#define OK "OK\n"
#define ERROR "ERROR\n"

int main(int argc, char **argv) {
    my_args = parse_args(argc, argv);
    if (check_args(my_args) == V_UNKNOWN) {
        exit(1);
    }
    sock_fd = connect_to_server(my_args[0], my_args[1]);
    if (sock_fd == 1 || write_client_request(check_args(my_args)) == 1 || execute_request(check_args(my_args)) == 1) {
        exit(1);
    }
    shutdown(sock_fd, SHUT_RD);
    close(sock_fd);
    free(my_args);
    return 0;
}

int connect_to_server(char *host, char *port) {
    struct addrinfo hints = {0}, *result = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &hints, &result) != 0) {
        perror("err");
        return 1;
    }
    int sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock_fd == -1 || connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
        perror("err");
        close(sock_fd);
        freeaddrinfo(result);
        return 1;
    }
    freeaddrinfo(result);
    return sock_fd;
}

int write_client_request(verb request) {
    char* s = NULL;
    if (request == LIST) {
        s = malloc(strlen(my_args[2]) + 2);
        sprintf(s, "%s\n", my_args[2]);
    } else {
        s = malloc(strlen(my_args[2])+strlen(my_args[3])+3);
        sprintf(s, "%s %s\n", my_args[2], my_args[3]);
    }
    if (!s) {
        perror("err");
        return 1;
    }
    ssize_t write_count = write_to_socket(sock_fd, s, strlen(s));
    if (write_count < 0) {
        perror("err");
        return 1;
    }
    if ((size_t)write_count < strlen(s)) {
        print_connection_closed();
        return 1;
    }
    free(s);
    return 0;
}

int printing_errors(size_t read_bytes, size_t size) {
    if (read_bytes == 0 && read_bytes != size) {
        print_connection_closed();
        return 1;
    } else if (read_bytes < size) {
        print_too_little_data();
        return 1;
    } else if (read_bytes > size) {
        print_received_too_much_data();
        return 1;
    }
    return 0;
}

int execute_request(verb request) {
    if (request == PUT) {
        if (handle_put() != 0) {
            return 1;
        }
    }
    if (shutdown(sock_fd, SHUT_WR) != 0) {
        perror("err");
        return 1;
    }
    char* buffer = calloc(1, strlen(OK) + 1);
    if (!buffer) {
        perror("err");
        return 1;
    }
    size_t read_bytes = read_from_socket(sock_fd, buffer, strlen(OK));
    if (strcmp(buffer, OK) != 0) {
        char* new_buffer = realloc(buffer, strlen(ERROR) + 1);
        if (!new_buffer) {
            perror("err");
            free(buffer);
            return 1;
        }
        read_from_socket(sock_fd, new_buffer + read_bytes, strlen(ERROR) - read_bytes);
        if (strcmp(new_buffer, ERROR) == 0) {
            char err[24] = {0};
            if (read_from_socket(sock_fd, err, 24) == 0) {
                print_connection_closed();
            }
            print_error_message(err);
        } else {
            print_invalid_response();
        }

        free(new_buffer);
        return 1;
    }
    if (request == LIST) {
        size_t size;
        read_from_socket(sock_fd, (char*)&size, sizeof(size_t));
        char list_buffer[size + 6];
        memset(list_buffer, 0, size + 6);
        read_bytes = read_from_socket(sock_fd, list_buffer, size + 5);

        if (printing_errors(read_bytes, size) == 0) {
            fprintf(stdout, "%s\n", list_buffer);
        } else {
            free(buffer);
            return 1;
        }
    } else if (request == GET) {
        FILE *local_file = fopen(my_args[4], "a+");
        if (!local_file) {
            perror("err");
            free(buffer);
            return 1;
        }
        size_t buff_size;
        read_from_socket(sock_fd, (char *)&buff_size, sizeof(size_t));
        size_t read_bytes = 0;
        size_t r_size;
        while (read_bytes < buff_size + 4) {
            if ((buff_size + 4 - read_bytes) > 1024){
                r_size = 1024;
            } else {
                r_size = buff_size + 4 - read_bytes;
            }
            char get_buffer[1025] = {0};
            size_t read_count = read_from_socket(sock_fd, get_buffer, r_size);
            fwrite(get_buffer, 1, read_count, local_file);
            read_bytes = read_bytes + read_count;

            if (read_count == 0) {
                break;
            }
        }
        if (printing_errors(read_bytes, buff_size) != 0) {
            fclose(local_file);
            free(buffer);
            return 1;
        }
        fclose(local_file);
    } else if (request == PUT || request == DELETE) {
        print_success();
    }
    free(buffer);
    return 0;
}

int handle_put() {
    struct stat statbuf;
    if (stat(my_args[4], &statbuf) == -1) {
        perror("err");
        return 1;
    }
    size_t stat_size = statbuf.st_size;
    if (write_to_socket(sock_fd, (char*)&stat_size, sizeof(size_t)) < 0) {
        perror("err");
        return 1;
    }
    FILE* local_file = fopen(my_args[4], "rb");
    if (!local_file) {
        perror("err");
        return 1;
    }
    size_t w_total = 0;
    char buffer[1025] = {0};
    ssize_t read_count;
    while ((read_count = fread(buffer, 1, sizeof(buffer) - 1, local_file)) > 0) {
        ssize_t write_count = write_to_socket(sock_fd, buffer, read_count);
        if (write_count < read_count) {
            print_connection_closed();
            fclose(local_file);
            return 1;
        }
        w_total += write_count;
    }
    fclose(local_file);
    if (w_total < stat_size) {
        print_connection_closed();
        return 1;
    }
    return 0;
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }
    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }
    char **my_args = calloc(1, 6 * sizeof(char *));
    my_args[0] = host;
    my_args[1] = port;
    my_args[2] = argv[2];
    char *temp = my_args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        my_args[3] = argv[3];
    }
    if (argc > 4) {
        my_args[4] = argv[4];
    }
    return my_args;
}

/**
 * Validates my_args to program.  If `my_args` are not valid, help information for the
 * program is printed.
 *
 * my_args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **my_args) {
    if (my_args == NULL) {
        print_client_usage();
        exit(1);
    }
    char *command = my_args[2];
    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }
    if (strcmp(command, "GET") == 0) {
        if (my_args[3] != NULL && my_args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }
    if (strcmp(command, "DELETE") == 0) {
        if (my_args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }
    if (strcmp(command, "PUT") == 0) {
        if (my_args[3] == NULL || my_args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }
    // Not a valid Method
    print_client_help();
    exit(1);
}