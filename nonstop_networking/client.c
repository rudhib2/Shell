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
int connection(char* host, char* port);
int req(verb request);
int cli_req(verb request);
int putt();
static char** my_args;
static int sock_fd;
#define OK "OK\n"
#define ERROR "ERROR\n"

int main(int argc, char **argv) {
    my_args = parse_args(argc, argv);
    if (check_args(my_args) == V_UNKNOWN) {
        exit(1);
    }
    sock_fd = connection(my_args[0], my_args[1]);
    if (sock_fd == 1 || cli_req(check_args(my_args)) == 1 || req(check_args(my_args)) == 1) {
        exit(1);
    }
    shutdown(sock_fd, SHUT_RD);
    close(sock_fd);
    free(my_args);
    return 0;
}

int connection(char *host, char *port) {
    struct addrinfo h = {0}, *result = NULL;
    h.ai_family = AF_INET;
    h.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &h, &result) != 0) {
        perror("err");
        return 1;
    }
    int fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (fd == -1 || connect(fd, result->ai_addr, result->ai_addrlen) == -1) {
        perror("err");
        close(fd);
        freeaddrinfo(result);
        return 1;
    }
    freeaddrinfo(result);
    return fd;
}

int cli_req(verb request) {
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
    ssize_t wcount = write_to_socket(sock_fd, s, strlen(s));
    if (wcount < 0) {
        perror("err");
        return 1;
    }
    if ((size_t)wcount < strlen(s)) {
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

int req(verb request) {
    if (request == PUT) {
        if (putt() != 0) {
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
    size_t b = read_from_socket(sock_fd, buffer, strlen(OK));
    if (strcmp(buffer, OK) != 0) {
        char* buff = realloc(buffer, strlen(ERROR) + 1);
        if (!buff) {
            perror("err");
            free(buffer);
            return 1;
        }
        read_from_socket(sock_fd, buff + b, strlen(ERROR) - b);
        if (strcmp(buff, ERROR) == 0) {
            char err[24] = {0};
            if (read_from_socket(sock_fd, err, 24) == 0) {
                print_connection_closed();
            }
            print_error_message(err);
        } else {
            print_invalid_response();
        }
        free(buff);
        return 1;
    }
    if (request == LIST) {
        size_t size;
        read_from_socket(sock_fd, (char*)&size, sizeof(size_t));
        char list_buffer[size + 6];
        memset(list_buffer, 0, size + 6);
        b = read_from_socket(sock_fd, list_buffer, size + 5);
        if (printing_errors(b, size) == 0) {
            fprintf(stdout, "%s\n", list_buffer);
        } else {
            free(buffer);
            return 1;
        }
    } else if (request == GET) {
        FILE *filee = fopen(my_args[4], "a+");
        if (!filee) {
            perror("err");
            free(buffer);
            return 1;
        }
        size_t b_size;
        read_from_socket(sock_fd, (char *)&b_size, sizeof(size_t));
        size_t read_bytes = 0;
        size_t r_size;
        while (read_bytes < b_size + 4) {
            if ((b_size + 4 - read_bytes) > 1024){
                r_size = 1024;
            } else {
                r_size = b_size + 4 - read_bytes;
            }
            char get_buffer[1025] = {0};
            size_t r_count = read_from_socket(sock_fd, get_buffer, r_size);
            fwrite(get_buffer, 1, r_count, filee);
            read_bytes = read_bytes + r_count;
            if (r_count == 0) {
                break;
            }
        }
        if (printing_errors(read_bytes, b_size) != 0) {
            fclose(filee);
            free(buffer);
            return 1;
        }
        fclose(filee);
    } else if (request == PUT || request == DELETE) {
        print_success();
    }
    free(buffer);
    return 0;
}

int putt() {
    struct stat statbuf;
    if (stat(my_args[4], &statbuf) == -1) {
        perror("err");
        return 1;
    }
    size_t s = statbuf.st_size;
    if (write_to_socket(sock_fd, (char*)&s, sizeof(size_t)) < 0) {
        perror("err");
        return 1;
    }
    FILE* filee = fopen(my_args[4], "rb");
    if (!filee) {
        perror("err");
        return 1;
    }
    size_t w_total = 0;
    char buffer[1025] = {0};
    ssize_t r_count;
    while ((r_count = fread(buffer, 1, sizeof(buffer) - 1, filee)) > 0) {
        ssize_t w_count = write_to_socket(sock_fd, buffer, r_count);
        if (w_count < r_count) {
            print_connection_closed();
            fclose(filee);
            return 1;
        }
        w_total += w_count;
    }
    fclose(filee);
    if (w_total < s) {
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