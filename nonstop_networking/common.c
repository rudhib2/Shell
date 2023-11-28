//used chatGPT for debugging and initial structure

/**
 * nonstop_networking
 * CS 341 - Fall 2023
 */
#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
// static const size_t MESSAGE_SIZE_DIGITS = 4;
int print_any_err(size_t bytes_rd, size_t size) {
    int result = 0;
    switch (bytes_rd) {
        case 0:
            if (bytes_rd != size) {
                print_connection_closed();
                result = 1;
            }
            break;
        case 1:
            break;
        default:
            if (bytes_rd < size) {
                print_too_little_data();
                result = 1;
            } else {
                print_received_too_much_data();
                result = 1;
            }
    }
    return result;
}

ssize_t read_from_socket(int socket, char *buffer, size_t count) {
    size_t bytes_received = 0;
    for (; bytes_received < count;) {
        ssize_t ret = read(socket, buffer + bytes_received, count - bytes_received);
        if (ret == 0) {
            return bytes_received;  
        } else if (ret > 0) {
            bytes_received += ret;
        } else if (ret == -1) {
            if (errno == EINTR) {
                continue;  
            } else {
                return -1;  
            }
        }
    }
    return bytes_received;
}


ssize_t write_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    size_t bytes_written = 0;
    while (bytes_written < count) {
        ssize_t ret = write(socket, buffer + bytes_written, count - bytes_written);
        if (ret < 0) {
            if (errno == EINTR) {
                continue; 
            } else {
                return -1; 
            }
        } else if (ret == 0) {
            return bytes_written; 
        }
        bytes_written += ret;
    }
    return bytes_written;
}


ssize_t read_header_from_socket(int socket, char *buffer, size_t count) {
    size_t return_code = 0;

    while (return_code < count) {

        ssize_t read_code = read(socket, (void*) (buffer + return_code), 1);
        if (read_code == -1 && errno == EINTR) {
            continue;
        }

        if (read_code == 0 || buffer[strlen(buffer) - 1] == '\n') {
            break;
        }

        if (read_code == -1) {
            return 1;
        }
        return_code += read_code;
    }

    return return_code;
}