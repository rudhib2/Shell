/**
 * charming_chatroom
 * CS 341 - Fall 2023
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
    size_t nsize = htonl(size);
    return write_all_to_socket(socket, (char*)&nsize, MESSAGE_SIZE_DIGITS);
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
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


ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    size_t bytes_written = 0;
    
    while (bytes_written < count) {
        ssize_t ret = write(socket, buffer + bytes_written, count - bytes_written);
        if (ret < 0) {
            if (errno == EINTR) {
                continue; // Interrupted, continue writing
            } else {
                return -1; // Error, return -1
            }
        } else if (ret == 0) {
            return bytes_written; // Connection closed, return bytes written so far
        }
        
        bytes_written += ret;
    }
    
    return bytes_written;
}