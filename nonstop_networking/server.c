// /**
//  * nonstop_networking
//  * CS 341 - Fall 2023
//  */

// used ChatGPT for initial desing and debugging - worked well!
#include "format.h"
#include "common.h"
#include "includes/dictionary.h"
#include "includes/vector.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <dirent.h>

typedef struct ConnectState {
	verb command;
	char server_filename[25];
    char header[1024];
    int status;
} ConnectState;

int process_header(ConnectState* connec, int client);
int process_command(ConnectState* connect, int client);
int send_file_list(int client);
void epoll_setup();
void server_closed();
void cleanup();
int send_file(ConnectState* connect, int client);
int put1(ConnectState* connect, int client);
int delete_file(ConnectState* connect, int client);
int setup_directory();
int removeDirectory(const char* path);
void modify_epoll_events(int fd, uint32_t events);
void setup_server_socket(const char *port);
void close_server_socket();

static vector* serverfile;
static char* p;
static dictionary* filesizeofserver;
static dictionary* dictionary_of_client;

int main(int argc, char **argv) {
    if (argc != 2){
        print_server_usage();
        return 1;  
    }
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    act.sa_flags = SA_RESTART;
    if (sigaction(SIGPIPE, &act, NULL)) {
        perror("Error setting up SIGPIPE handler");
        return 1;
    }
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = server_closed;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up SIGINT handler");
        return 1;
    }
    setup_directory();
    p = strdup(argv[1]);
    dictionary_of_client = int_to_shallow_dictionary_create();
    filesizeofserver = string_to_unsigned_long_dictionary_create();
    serverfile = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    setup_server_socket(p);
    epoll_setup();
    close_server_socket();
    return 0;
}

int removeDirectory(const char* path) {
    struct stat fileStat;
    if (stat(path, &fileStat) == 0) {
        if (S_ISDIR(fileStat.st_mode)) {
            DIR* d = opendir(path);
            if (!d) {
                perror("error");
                return 1;
            }
            struct dirent* e;
            char subPath[500] = {0};
            while ((e = readdir(d))) {
                sprintf(subPath, "%s/%s", path, e->d_name);
                if (!(strcmp(e->d_name, ".")) || !(strcmp(e->d_name, ".."))) {
                    continue;
                }
                if (unlink(subPath)) {
                    perror("error");
                    return 1;
                } else if (S_ISDIR(fileStat.st_mode)) {
                    if (removeDirectory(subPath)) {
                        return 1;
                    }
                }
            }
            if (closedir(d) || rmdir(path)) {
                perror("error");
                return 1;
            }
            return 0;
        } else {
            fprintf(stderr, "Error: %s is not a directory\n", path);
            return 1;
        }
    } else {
        perror("error");
        return 1;
    }
}

static char* temporary_directory;
static int fd_;

void cleanup() {
    close(fd_);
    vector_destroy(serverfile);
    vector *clientInfos = dictionary_values(dictionary_of_client);
    VECTOR_FOR_EACH(clientInfos, info, {
        free(info);
    });
    vector_destroy(clientInfos);
    dictionary_destroy(dictionary_of_client);
    dictionary_destroy(filesizeofserver);
    if (rmdir(temporary_directory) != 0 && removeDirectory(temporary_directory) != 0) {
        perror("error");
    }
}

void server_closed() {
    cleanup();
    exit(1);
}

void modify_epoll_events(int fd, uint32_t events) {
    struct epoll_event event_e;
    event_e.events = events;
    event_e.data.fd = fd;
    epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &event_e);
}

int setup_directory() {
    char template[] = "XXXXXX";
    temporary_directory = mkdtemp(template);
    if (temporary_directory == NULL) {
        perror("Error creating temporary directory");
        return 1; 
    }
    print_temp_directory(temporary_directory);
    return 0; 
}

static int sock;
void setup_server_socket(const char *port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    struct addrinfo hint, *res;
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_family = AF_INET;
    hint.ai_flags = AI_PASSIVE;
    int getAddrInfoResult = getaddrinfo(NULL, port, &hint, &res);
    if (getAddrInfoResult != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getAddrInfoResult));
        if (res != NULL) {
            freeaddrinfo(res);
        }
        exit(EXIT_FAILURE);
    }
    int value = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) != 0 ||
        bind(sock, res->ai_addr, res->ai_addrlen) == -1 ||
        listen(sock, 10) == -1) {
        perror("Error setting up server socket");
        if (res != NULL) {
            freeaddrinfo(res);
        }
        close(sock);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
}

void close_server_socket() {
    close(sock);
}

void epoll_setup() {
    fd_ = epoll_create(101);
    if (fd_ == -1) {
        perror("Error creating epoll");
        exit(EXIT_FAILURE);
    }
    struct epoll_event ep;
    ep.data.fd = sock;
    ep.events = EPOLLIN;
    if (epoll_ctl(fd_, EPOLL_CTL_ADD, sock, &ep) == -1) {
        perror("Error adding server socket to epoll");
        exit(EXIT_FAILURE);
    }
    struct epoll_event events[100];
    while (true) {
        int num = epoll_wait(fd_, events, 100, -1);
        if (num == 0) {
            if (num == -1) {
                perror("Error in epoll_wait");
                exit(EXIT_FAILURE);
            }
            continue;
        }
        for (int i = 0; i < num; i++) {
            int fd = events[i].data.fd;
            if (fd == sock) {
                int client = accept(sock, NULL, NULL);
                if (client == -1) {
                    perror("Error accepting client connection");
                    exit(EXIT_FAILURE);
                }
                struct epoll_event event2;
                event2.events = EPOLLIN;
                event2.data.fd = client;
                if (epoll_ctl(fd_, EPOLL_CTL_ADD, client, &event2) == -1) {
                    perror("Error adding client socket to epoll");
                    exit(EXIT_FAILURE);
                }
                ConnectState* connect = calloc(1, sizeof(ConnectState));
                connect->status = 0;
                dictionary_set(dictionary_of_client, &client, connect);
            } else {
                ConnectState* connect_client = dictionary_get(dictionary_of_client, &fd);
                if (connect_client->status == 0 && process_header(connect_client, fd) == 1) {
                    return;
                } else if (connect_client->status == 1) {
                    process_command(connect_client, fd);
                }
            }
        }
    }
}

int process_header(ConnectState* connect, int client) {
    char head[1024] = {0};
    if (read_header_from_socket(client, head, 1024) == 1024) {
        write_to_socket(client, err_bad_request, strlen(err_bad_request));
        modify_epoll_events(client, EPOLLOUT);
        return 1;
    }
    if (!strncmp(head, "LIST", 4)) {
        if (strcmp(head, "LIST\n") != 0) {
            print_invalid_response();
            modify_epoll_events(client, EPOLLOUT);
            return 1;
        }
        connect->command = LIST;
    } else if (!strncmp(head, "DELETE", 6)) {
        connect->command = DELETE;
        strncpy(connect->server_filename, head + 7, sizeof(connect->server_filename) - 1);
    } else if (!strncmp(head, "PUT", 3)) {
        connect->command = PUT;
        strncpy(connect->server_filename, head + 4, sizeof(connect->server_filename) - 1);
    } else if (!strncmp(head, "GET", 3)) {
        connect->command = GET;
        strncpy(connect->server_filename, head + 4, sizeof(connect->server_filename) - 1);
    } else {
        print_invalid_response();
        modify_epoll_events(client, EPOLLOUT);
        return 1;
    }
    if (connect->command != LIST) {
        connect->server_filename[strlen(connect->server_filename) - 1] = '\0';
    }
    modify_epoll_events(client, EPOLLOUT);
    connect->status = 1;
    return 0;
}

int delete_file(ConnectState* connect, int client) {
    size_t path_len = strlen(temporary_directory) + 1 + strlen(connect->server_filename) + 1;
    char path[path_len];
    memset(path, 0, path_len);
    sprintf(path, "%s/%s", temporary_directory, connect->server_filename);
    if (remove(path) == -1) {
        write_to_socket(client, err_no_such_file, strlen(err_no_such_file));
        perror("Error deleting file");
        return 1;
    }
    size_t i = 0;
    VECTOR_FOR_EACH(serverfile, filename, {
        if (strcmp((char*)filename, connect->server_filename) == 0) {
            vector_erase(serverfile, i);
            return 0;
        }
        i++;
    });
    write_to_socket(client, err_no_such_file, strlen(err_no_such_file));
    return 1;
}

int process_command(ConnectState* connect, int client) {
    int result = 0;
    switch (connect->command) {
        case GET:
            result = send_file(connect, client);
            break;
        case DELETE:
            result = delete_file(connect, client);
            if (result == 0) {
                write_to_socket(client, "OK\n", 3);
            }
            break;
        case LIST:
            write_to_socket(client, "OK\n", 3);
            result = send_file_list(client);
            break;
        case PUT:
            result = put1(connect, client);
            if (result == 0) {
                write_to_socket(client, "OK\n", 3);
            }
            break;
        default:
            //unknown command
            result = 1;
            break;
    }
    epoll_ctl(fd_, EPOLL_CTL_DEL, client, NULL);
    shutdown(client, SHUT_RDWR);
    close(client);
    return result;
}

int send_file_list(int client) {
    size_t total_length = 0;
    VECTOR_FOR_EACH(serverfile, file, {
        total_length += strlen(file) + 1;
    });
    if (total_length > 0) {
        total_length--;
    }
    write_to_socket(client, (char*)&total_length, sizeof(size_t));
    VECTOR_FOR_EACH(serverfile, file, {
        write_to_socket(client, file, strlen(file));
        if (_it + 1 != _iend) {
            write_to_socket(client, "\n", 1);
        }
    });
    return 0;
}

int put1(ConnectState* connect, int client) {
    size_t l = strlen(temporary_directory) + 1 + strlen(connect->server_filename) + 1;
    char path_of_file[l];
    memset(path_of_file , 0, l);
    sprintf(path_of_file, "%s/%s", temporary_directory, connect->server_filename);
    FILE* read = fopen(path_of_file, "rb");
    FILE* write = fopen(path_of_file, "wb");
    if (!write) {
        perror("error");
        return 1;
    }
    size_t ff;
    read_from_socket(client, (char*) &ff, sizeof(size_t));
    size_t bytes = 0;
    while (bytes < ff + 4) {
        size_t sizeofheader;
        if ((ff + 4 - bytes) <= 1024) {
            sizeofheader = ff + 4 - bytes;
        } else {
            sizeofheader = 1024;
        }
        char buff[1025];
        memset(buff, 0, sizeof(buff));
        ssize_t c = read_from_socket(client, buff, sizeofheader);
        if (c == -1) {
            continue;
        } 
        fwrite(buff, 1, c, write);
        bytes = bytes + c;
        if (c == 0) {
            break;
        } 
    }
    if (read) {
        fclose(read);
    } else {
        vector_push_back(serverfile, connect->server_filename);
    }
    fclose(write);
    dictionary_set(filesizeofserver, connect->server_filename, &ff);
    return 0;
}

int send_file(ConnectState* connect, int client) {
    size_t path_length = strlen(temporary_directory) + 1 + strlen(connect->server_filename) + 1;
    char file_path[path_length];
    memset(file_path, 0, path_length);
    sprintf(file_path, "%s/%s", temporary_directory, connect->server_filename);
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        write_to_socket(client, err_no_such_file, strlen(err_no_such_file));
        perror("Error opening file for reading");
        return 1;
    }
    write_to_socket(client, "OK\n", 3);
    size_t size_of_file = *(size_t*)dictionary_get(filesizeofserver, connect->server_filename);
    write_to_socket(client, (char*)&size_of_file, sizeof(size_t));
    size_t count = 0;
    size_t chunk_size;
    while (count < size_of_file) {
        chunk_size = (size_of_file - count) <= 1024 ? (size_of_file - count) : 1024;
        char buffer[1024];
        fread(buffer, 1, chunk_size, file);
        write_to_socket(client, buffer, chunk_size);
        count += chunk_size;
    }
    fclose(file);
    return 0;
}