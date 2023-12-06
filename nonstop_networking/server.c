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

int dir (const char* path);
int noemptyrmdir(char* dir);
void connection1();
int rheader(ConnectState* connec, int client);
int command1(ConnectState* connect, int client);
int list1(int client);
void epoll_setup();
void server_closed();
int get1(ConnectState* connect, int client);
int put1(ConnectState* connect, int client);
int delete1(ConnectState* connect, int client);
void monitoring_epoll(int fd);
void setup_directory();

static vector* serverfile;
static char* p;
static dictionary* filesizeofserver;
static dictionary* dictionary_of_client;

int main(int argc, char **argv) {
    if (argc != 2){
        print_server_usage();
        exit(1);  
    }
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    act.sa_flags = SA_RESTART;
    if ( sigaction(SIGPIPE, &act, NULL)) {
        perror("err");
        exit(1);
    }
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = server_closed;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("err");
        exit(1);
    }
    setup_directory();
    p = strdup(argv[1]);
    dictionary_of_client = int_to_shallow_dictionary_create();
    filesizeofserver = string_to_unsigned_long_dictionary_create();
    serverfile = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    connection1();
    epoll_setup();
    server_closed();
}

int diry (const char* path) {
    struct stat s;
    if (!stat(path, &s)) {
        return S_ISDIR(s.st_mode);
    } else {
        return 0;
    }
}

int noemptyrmdir(char* dir) {
    DIR* d = opendir(dir);
    if (!d) {
        perror("error");
        return 1;
    }
    struct dirent* e;
    char b[500] = {0};
    while ((e = readdir(d))) {
        sprintf(b, "%s/%s", dir, e->d_name);
        if (!(strcmp(e->d_name, "."))) {
            continue;
        } 
        if (!(strcmp(e->d_name, ".."))) {
            continue;
        } 
        if (unlink(b)) {
            perror("error");
            return 1;
        } else if (diry(b) != 0) {
            noemptyrmdir(b);
        }
    }
    if (closedir(d) || rmdir(dir)) {
        perror("error");
        return 1;
    }
    return 0;
}

static char* temporary_directory;
static int fd_;
void server_closed() {
    close(fd_);
    vector_destroy(serverfile);
    vector *in = dictionary_values(dictionary_of_client);
    VECTOR_FOR_EACH(in, info, {
        free(info);
    });
    dictionary_destroy(dictionary_of_client);
    dictionary_destroy(filesizeofserver);
    if (rmdir(temporary_directory) != 0 && noemptyrmdir(temporary_directory) != 0) {
        perror("error");
    }
    exit(1);
}

void monitoring_epoll(int fd) {
    struct epoll_event event_e;
    event_e.events = EPOLLOUT; 
    event_e.data.fd = fd;
    epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &event_e);
}

void setup_directory() {
    char template[] = "XXXXXX";
    temporary_directory = mkdtemp(template);
    print_temp_directory(temporary_directory);
}

static int sock;
void connection1() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("error");
        exit(1);
    }
    struct addrinfo hint, *res;
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_family = AF_INET;
    hint.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, p, &hint, &res) != 0) {
        fprintf(stderr, "%s", gai_strerror(getaddrinfo(NULL, p, &hint, &res)));
        if (res != 0) {
            freeaddrinfo(res);
        } 
        exit(1);
    }
    int value = 1;
    if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value))) || (bind(sock, res->ai_addr, res->ai_addrlen) == -1) || (listen(sock, 10) == -1)) {
        perror("error");
        if (res != 0) {
            freeaddrinfo(res);
        } 
        exit(1);
    }
}

void epoll_setup() {
    fd_ = epoll_create(101);
    if (fd_ == -1) {
        perror("error");
        exit(1);
    }
    struct epoll_event ep;
    ep.data.fd = sock;
    ep.events = EPOLLIN;
    if (epoll_ctl(fd_, EPOLL_CTL_ADD, sock, &ep) == -1) {
        perror("error");
        exit(1);
    }
    struct epoll_event events[100];
    while (true) {
        int num = epoll_wait(fd_, events, 100, -1);
        if (num == 0) {
            continue;
        } else if (num == -1) {
            perror("error");
            exit(1);
        }
        for (int i = 0; i < num; i++) {
            int xy = events[i].data.fd;
            if (xy == sock) {
                int client = accept(sock, NULL, NULL);  
                if (client == -1) {
                    perror("error");
                    exit(1);
                }
                struct epoll_event event2;
                event2.events = EPOLLIN; 
                event2.data.fd = client;
                if (epoll_ctl(fd_, EPOLL_CTL_ADD, client, &event2) == -1) {
                    perror("error");
                    exit(1);
                }
                ConnectState* connect = calloc(1, sizeof(ConnectState));
                connect->status = 0;
                dictionary_set(dictionary_of_client, &client, connect);
            } else {               
                ConnectState* connect_client = dictionary_get(dictionary_of_client, &xy);
                if (connect_client->status == 0 && (rheader(connect_client, xy) == 1)) { 
                    return;
                } else if (connect_client->status == 1) {  
                    command1(connect_client, xy);
                }
            }
        }
    }
}

int rheader(ConnectState* connect, int client) {
    char head[1024] = {0};
    if ((read_header_from_socket(client, head, 1024)) == 1024) {
        write_to_socket(client, err_bad_request, strlen(err_bad_request));
        monitoring_epoll(client);
        return 1;
    }
    if (!strncmp(head, "LIST", 4)) {
        if (strcmp(head, "LIST\n") != 0) {
            print_invalid_response();
            monitoring_epoll(client);
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
        monitoring_epoll(client);
        return 1;
    }
    if (connect->command != LIST) {
        connect->server_filename[strlen(connect->server_filename) - 1] = '\0';
    }
    monitoring_epoll(client);
    connect->status = 1;
    return 0;
}

int delete1(ConnectState* connect, int client) {
    int l = strlen(temporary_directory) + 1 + strlen(connect->server_filename) + 1;
    char path[l];
    memset(path , 0, l);
    sprintf(path, "%s/%s", temporary_directory, connect->server_filename);
    if (remove(path) == -1) {
        write_to_socket(client, err_no_such_file, strlen(err_no_such_file));
        perror("error");
        return 1;
    }
    size_t i = 0;
    VECTOR_FOR_EACH(serverfile, filename, {
        if (strcmp((char *) filename, connect->server_filename) == 0) {
            vector_erase(serverfile, i);
            return 0;
        }
        i = i + 1;
    });
    write_to_socket(client, err_no_such_file, strlen(err_no_such_file));
    return 1;
}

int command1(ConnectState* connect, int client) {
    if (connect->command == GET && (get1(connect, client) != 0)) {
        return 1;
    }
    if (connect->command == DELETE) {
        if (delete1(connect, client) != 0) {
            return 1;
        }
        write_to_socket(client, "OK\n", 3);
    }
    if (connect->command == LIST) {
        write_to_socket(client, "OK\n", 3);
        if (list1(client) != 0) {
            return 1;
        }
    }
    if (connect->command == PUT) {
        if (put1(connect, client) != 0) {
            return 1;
        }
        write_to_socket(client, "OK\n", 3);
    }
    epoll_ctl(fd_, EPOLL_CTL_DEL, client, NULL);
    shutdown(client, SHUT_RDWR);
    close(client);
    return 0;
}

int list1(int client) {
    size_t s = 0;
    VECTOR_FOR_EACH(serverfile, file, {
        s = s + strlen(file) + 1;
    });
    if (s > 0) {
        s = s - 1;
    } 
    write_to_socket(client, (char*) &s, sizeof(size_t));
    VECTOR_FOR_EACH(serverfile, file, {
        write_to_socket(client, file, strlen(file));
        if (_it+1 != _iend) {
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
    if (!read) {
        vector_push_back(serverfile, connect->server_filename);
    } else {
        fclose(read);
    }
    fclose(write);
    dictionary_set(filesizeofserver, connect->server_filename, &ff);
    return 0;
}

// int get1(ConnectState* connect, int client) {
//     size_t l = strlen(temporary_directory) + 1 + strlen(connect->server_filename) + 1;
//     char file_path[l];
//     memset(file_path , 0, l);
//     sprintf(file_path, "%s/%s", temporary_directory, connect->server_filename);
//     FILE* filee = fopen(file_path, "rb");   
//     if(!filee) {
//         write_to_socket(client, err_no_such_file, strlen(err_no_such_file));
//         perror("error");
//         exit(1);
//     }
//     write_to_socket(client, "OK\n", 3);
//     size_t size_of_file = *(size_t*)dictionary_get(filesizeofserver, connect->server_filename);
//     write_to_socket(client, (char*)&size_of_file, sizeof(size_t));
//     size_t count = 0;
//     size_t head;
//     while (count < size_of_file) {
//         if ((size_of_file - count) <= 1024 ){
//             head = size_of_file - count;
//         } else {
//              head = 1024;
//         }
//         char buffer[head + 1];
//         fread(buffer, 1, head, filee);
//         write_to_socket(client, buffer, head);
//         count = count + head;
//     }
//     fclose(filee);
//     return 0;
// }
int get1(ConnectState* connect, int client) {
    size_t l = strlen(temporary_directory) + 1 + strlen(connect->server_filename) + 1;
    char file_path[l];
    memset(file_path , 0, l);
    sprintf(file_path, "%s/%s", temporary_directory, connect->server_filename);
    FILE* filee = fopen(file_path, "rb");   
    if (!filee) {
        write_to_socket(client, err_no_such_file, strlen(err_no_such_file));
        perror("error");
        return 1;
    }
    write_to_socket(client, "OK\n", 3);
    size_t size_of_file = *(size_t*)dictionary_get(filesizeofserver, connect->server_filename);
    write_to_socket(client, (char*)&size_of_file, sizeof(size_t));
    size_t count = 0;
    size_t chunk_size;
    while (count < size_of_file) {
        if ((size_of_file - count) <= 1024) {
            chunk_size = size_of_file - count;
        } else {
             chunk_size = 1024;
        }
        char buffer[chunk_size];
        fread(buffer, 1, chunk_size, filee);
        write_to_socket(client, buffer, chunk_size);
        count = count + chunk_size;
    }
    fclose(filee);
    return 0;
}