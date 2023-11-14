//used chatGPT for debugging, initial structure, and for generating functions

/**
 * nonstop_networking
 * CS 341 - Fall 2023
 */
#include "format.h"
#include <stdbool.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "common.h"

char **parse_args(int argc, char **argv);
verb check_args(char **args);
int connect_to_server(char **args);
void write_cmd(char **args, int socket, verb method);
void read_response(char **args, int socket, verb method);
void handleGetOperation(char **arguments, int socket);
void handleListOperation(int socket);
void handleErrorResponse(int socket);
void handleSuccessResponse(verb operation, char **arguments, int socket);
void sendListRequest(int socket, const char *arg);
void sendPutRequest(int socket, const char *arg1, const char *arg2, const char *filename);
void write_cmd(char **args, int socket, verb method);
int createSocket(struct addrinfo *result);
void establishConnection(int sock_fd, struct addrinfo *result);
void freeAddressInfo(struct addrinfo *result);

int main(int argc, char **argv) {
    // Good luck!
    char **args = parse_args(argc, argv);
    if (args == NULL) {
        return 1;
    }
    verb method = check_args(args);
    int serverSocket = connect_to_server(args);
    if (serverSocket < 0) {
        free(args);
        return 1;
    }
    write_cmd(args, serverSocket, method);
    if (shutdown(serverSocket, SHUT_WR) != 0) {
        perror("shutdown()");
    }
    read_response(args, serverSocket, method);
    shutdown(serverSocket, SHUT_RD);
    close(serverSocket);
    free(args);
}

struct addrinfo* getAddressInfo(const char *host, const char *port) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host, port, &hints, &result) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo(host, port, &hints, &result)));
        exit(1);
    }
    return result;
}



void handleGetOperation(char **arguments, int socket) {
    FILE *local_file = fopen(arguments[4], "a+");
    if (!local_file) {
        perror(NULL);
        exit(1);
    }
    size_t file_size_received;
    read_from_socket(socket, (char *)&file_size_received, sizeof(size_t));

    size_t total_bytes_read = 0;
    while (total_bytes_read < file_size_received + 5) {
        size_t remaining_bytes = file_size_received + 5 - total_bytes_read;
        remaining_bytes = (size_t)fmin((double)remaining_bytes, (double)1024);
        char data_chunk[1024 + 1] = {0};
        size_t bytes_read = read_from_socket(socket, data_chunk, remaining_bytes);
        fwrite(data_chunk, 1, bytes_read, local_file);
        total_bytes_read = total_bytes_read + bytes_read;
        if (bytes_read == 0) {
            break;
        }
    }
    if (print_any_err(total_bytes_read, file_size_received)) {
        exit(1);
    }
    fclose(local_file);
}


void handleListOperation(int socket) {
    size_t size;
    read_from_socket(socket, (char *)&size, sizeof(size_t));
    char data_buffer[size + 5 + 1];
    memset(data_buffer, 0, size + 5 + 1);
    size_t bytes_received = read_from_socket(socket, data_buffer, size + 5);
    if (print_any_err(bytes_received, size)) {
        exit(1);
    }
    fprintf(stdout, "%zu%s", size, data_buffer);
}

void handleErrorResponse(int socket) {
    char *response_buffer = calloc(1,strlen("OK\n")+1);
    size_t bytes_rd = read_from_socket(socket, response_buffer, strlen("OK\n"));
    char error_response[7] = "ERROR\n";
    size_t error_len = strlen(error_response);
    response_buffer = realloc(response_buffer, error_len + 1);
    read_from_socket(socket, response_buffer + bytes_rd, error_len - bytes_rd);

    if (strcmp(response_buffer, error_response) == 0) {
        printf("Received error: %s", response_buffer);
        char error_message[20] = {0};
        if (!read_from_socket(socket, error_message, 20)) {
            print_connection_closed();
        } else {
            print_error_message(error_message);
        }
    } else {
        print_invalid_response();
    }
    free(response_buffer);
}

void read_response(char **arguments, int socket, verb operation) {
    char expected_response[] = "OK\n";
    size_t response_length = strlen(expected_response) + 1;
    char *response_buffer = calloc(1, response_length);

    if (strcmp(response_buffer, expected_response) == 0) {
        fprintf(stdout, "%s", response_buffer);
        handleSuccessResponse(operation, arguments, socket);
    } else {
        handleErrorResponse(socket);
    }
    free(response_buffer);
}

void handleSuccessResponse(verb operation, char **arguments, int socket) {
    if (operation == GET) {
        handleGetOperation(arguments, socket);
    } else if (operation == PUT) {
        print_success();
    } else if (operation == DELETE) {
        print_success();
    } else if (operation == LIST) {
        handleListOperation(socket);
    }
}

void sendListRequest(int socket, const char *arg) {
    size_t arg_length = strlen(arg);
    char *msg = calloc(1, arg_length + 2);
    sprintf(msg, "%s\n", arg);
    
    if ((unsigned long) write_to_socket(socket, msg, strlen(msg)) < strlen(msg)) {
        print_connection_closed();
        exit(1);
    }
    free(msg);
}

void sendPutRequest(int socket, const char *arg1, const char *arg2, const char *filename) {
    size_t arg_length = strlen(arg1) + strlen(arg2) + 3;
    char *msg = calloc(1, arg_length);
    sprintf(msg, "%s %s\n", arg1, arg2);
    
    if ((unsigned long) write_to_socket(socket, msg, strlen(msg)) < strlen(msg)) {
        print_connection_closed();
        exit(1);
    }
    free(msg);

    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        exit(1);
    }

    size_t file_size = file_stat.st_size;
    if ((size_t)write_to_socket(socket, (char*)&file_size, sizeof(size_t)) < sizeof(size_t)) {
        print_connection_closed();
        exit(1);
    }

    FILE *local_file = fopen(filename, "r");
    if (!local_file) {
        fprintf(stdout, "Failed to open local file\n");
        exit(1);
    }

    size_t bytes_written = 0;
    while (bytes_written < file_size) {
        size_t remaining = (file_size - bytes_written) > 1024 ? 1024 : (file_size - bytes_written);
        char buffer[remaining];
        fread(buffer, 1, remaining, local_file);
        
        if ((size_t)write_to_socket(socket, buffer, remaining) < remaining) {
            print_connection_closed();
            exit(1);
        }
        bytes_written = bytes_written + remaining;
    }
    fclose(local_file);
}

void write_cmd(char **args, int socket, verb method) {
    if (method == LIST) {
        sendListRequest(socket, args[2]);
    } else {
        sendPutRequest(socket, args[2], args[3], args[4]);
    }
}

int createSocket(struct addrinfo *result) {
    int sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock_fd == -1) {
        perror(NULL);
        exit(1);
    }
    return sock_fd;
}

void establishConnection(int sock_fd, struct addrinfo *result) {
    if (connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
        perror(NULL);
        exit(1);
    }
}

void freeAddressInfo(struct addrinfo *result) {
    freeaddrinfo(result);
}

int connect_to_server(char **args) {
    struct addrinfo *result = getAddressInfo(args[0], args[1]);
    int sock_fd = createSocket(result);
    establishConnection(sock_fd, result);
    freeAddressInfo(result);
    return sock_fd;
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

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
