//used chatGPT for debugging and initial structure

/**
 * nonstop_networking
 * CS 341 - Fall 2023
 */
#include "common.h"

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
    size_t total_bytes = 0;
    while (total_bytes < count) {
        ssize_t bytes_read = read(socket, buffer + total_bytes, count - total_bytes);

        if (bytes_read <= 0) {
            if (bytes_read == 0 && total_bytes == 0) {
                print_connection_closed();
            } else if (bytes_read != -1 && errno == EINTR) {
                continue;
            } else if (bytes_read == -1) {
                perror("read_from_socket()");
            }
            break;
        }
        total_bytes += bytes_read;
    }
    return (total_bytes <= 0) ? -1 : total_bytes;
}

ssize_t write_to_socket(int socket, const char *buffer, size_t count) {
    size_t bytes_written = 0;
    while (bytes_written < count) {
        ssize_t ret = write(socket, buffer + bytes_written, count - bytes_written);
        if (ret <= 0) {
            if (ret == -1 && errno == EINTR) {
                continue;  
            } else {
                perror("write_to_socket()");
                return -1;
            }
        }
        bytes_written += ret;
    }
    return bytes_written;
}
