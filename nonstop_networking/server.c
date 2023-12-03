// /**
//  * nonstop_networking
//  * CS 341 - Fall 2023
//  */

// used ChatGPT for initial desing and debugging - worked well
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

void sigpipe_handler() {}
void setup_directory();
void setup_connection();
void setup_epoll();
void close_server();
int read_header(ConnectState* connection, int client_fd);
int execute_command(ConnectState* connection, int client_fd);
int execute_list(ConnectState* connection, int client_fd);
int execute_get(ConnectState* connection, int client_fd);
int execute_put(ConnectState* connection, int client_fd);
int execute_delete(ConnectState* connection, int client_fd);
void epoll_monitor(int fd);

static char* temp_dir_;
static char* port_;
static int epfd_;
static int sock_fd_;
static dictionary* client_dictionary_;
static dictionary* server_file_sizes_;
static vector* server_files_;


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
        perror("sigaction()");
        exit(1);
    }
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = close_server;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction()");
        exit(1);
    }
    setup_directory();
    port_ = strdup(argv[1]);
    client_dictionary_ = int_to_shallow_dictionary_create();
    server_file_sizes_ = string_to_unsigned_long_dictionary_create();
    server_files_ = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    setup_connection();
    setup_epoll();
    close_server();
}

int is_dir (const char* path) {
    struct stat s;
    if (stat(path, &s)) return 0;
    else return S_ISDIR(s.st_mode);
}

int rmdir_nonempty(char* dir) {
    DIR* dp = opendir(dir);
    struct dirent* de;
    char p_buf[500] = {0};
    while ((de = readdir(dp))) {
        sprintf(p_buf, "%s/%s", dir, de->d_name);
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        if (is_dir(p_buf)) {
            rmdir_nonempty(p_buf);
        } else {
            if (unlink(p_buf) != 0) {
                perror("unlink()");
                return 1;
            }
        }
    }
    if (closedir(dp) != 0) {
        perror("closedir()");
        return 1;
    }
    if (rmdir(dir) != 0) {
        perror("rmdir()");
        return 1;
    }
    return 0;
}

void close_server() {
    close(epfd_);
    vector_destroy(server_files_);
    vector *infos = dictionary_values(client_dictionary_);
	VECTOR_FOR_EACH(infos, info, {
    	free(info);
	});
    dictionary_destroy(client_dictionary_);
    dictionary_destroy(server_file_sizes_);
    if (rmdir(temp_dir_) != 0) {
        if (rmdir_nonempty(temp_dir_) != 0) {
            perror("couldn't remove temp directory");
        }
    }
    exit(1);
}

void epoll_monitor(int fd) {
    struct epoll_event event;
    event.events = EPOLLOUT; 
    event.data.fd = fd;
    epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
}

void setup_directory() {
    char template[] = "XXXXXX";
    temp_dir_ = mkdtemp(template);
    print_temp_directory(temp_dir_);
}

void setup_connection() {
    int getaddrinfo_res;
    sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd_ == -1) {
        perror("socket()\n");
        exit(1);
    }
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo_res = getaddrinfo(NULL, port_, &hints, &result);
    if (getaddrinfo_res != 0) {
        fprintf(stderr, "%s", gai_strerror(getaddrinfo_res));
        if (result) freeaddrinfo(result);
        exit(1);
    }
     int val = 1;
    if (setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val))) {
        perror("setsockopt()");
        if (result) freeaddrinfo(result);
        exit(1);
    }
    if (bind(sock_fd_, result->ai_addr, result->ai_addrlen) == -1) {
        perror("bind()");
        if (result) freeaddrinfo(result);
        exit(1);
    }
    if (listen(sock_fd_, 10) == -1) {
        perror("listen()");
        if (result) freeaddrinfo(result);
        exit(1);
    }
}

void setup_epoll() {
    epfd_ = epoll_create(101);
    if (epfd_ == -1) {
        perror("epoll_create()");
        exit(1);
    }
    struct epoll_event ep_event;
    ep_event.events = EPOLLIN;  
    ep_event.data.fd = sock_fd_;
    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, sock_fd_, &ep_event) == -1) {
        perror("epoll_ctl...()");
        exit(1);
    }
    struct epoll_event ep_events[100];
    while (true) {
        int num_fds = epoll_wait(epfd_, ep_events, 100, -1);
        if (num_fds == -1) {
            perror("epoll_wait()");
            exit(1);
        } else if (num_fds == 0) {
            continue; 
        }
        for (int i = 0; i < num_fds; i++) {
            int ep_fd = ep_events[i].data.fd;
            if (ep_fd == sock_fd_) {
                int client_fd = accept(sock_fd_, NULL, NULL); 
                if (client_fd == -1) {
                    perror("accept()");
                    exit(1);
                }
                struct epoll_event another_event;
                another_event.events = EPOLLIN;  
                another_event.data.fd = client_fd;
                if (epoll_ctl(epfd_, EPOLL_CTL_ADD, client_fd, &another_event) == -1) {
                    perror("epoll_ctl!()");
                    exit(1);
                }
                ConnectState* connection = calloc(1, sizeof(ConnectState));
                connection->status = 0;
                dictionary_set(client_dictionary_, &client_fd, connection);
            } else {               
                ConnectState* client_connection = dictionary_get(client_dictionary_, &ep_fd);
                if (client_connection->status == 0) { 
                    if (read_header(client_connection, ep_fd) == 1) {
                        return;
                    }
                } else if (client_connection->status == 1) {   
                    if (execute_command(client_connection, ep_fd) != 0) {
                    }
                }
            }
        }
    }

}

int read_header(ConnectState* connection, int client_fd) {
    char* header = calloc(1, sizeof(char));
    size_t br = read_header_from_socket(client_fd, header, 1024);
    if (br == 1024) {
        write_to_socket(client_fd, err_bad_request, strlen(err_bad_request));
        epoll_monitor(client_fd);
        return 1;
    }
    printf("header: %s\n", header);
    if (strncmp(header, "LIST", 4) == 0) {
        if (strcmp(header, "LIST\n") != 0) {
            print_invalid_response();
            epoll_monitor(client_fd);
        return 1;
        }
        connection->command = LIST;
    } else if (strncmp(header, "PUT", 3) == 0) {
        connection->command = PUT;
        strcpy(connection->server_filename, header + 4);
    } else if (strncmp(header, "GET", 3) == 0) {
        connection->command = GET;
        strcpy(connection->server_filename, header + 4);
    } else if (strncmp(header, "DELETE", 6) == 0) {
        connection->command = DELETE;
        strcpy(connection->server_filename, header + 7);
    } else {
        print_invalid_response();
        epoll_monitor(client_fd);
        return 1;
    }
    if (connection->command != LIST) {
        connection->server_filename[strlen(connection->server_filename) - 1] = '\0';
    }
    epoll_monitor(client_fd);
    connection->status = 1;
    return 0;
}

int execute_command(ConnectState* connection, int client_fd) {
    verb command = connection->command;
    if (command == GET) {
        if (execute_get(connection, client_fd) != 0) {
            return 1;
        }
        
    }
    if (command == PUT) {
        if (execute_put(connection, client_fd) != 0) {
            return 1;
        }
        write_to_socket(client_fd, "OK\n", 3);
    }

    if (command == LIST) {
        write_to_socket(client_fd, "OK\n", 3);
        if (execute_list(connection, client_fd) != 0) {
            return 1;
        }
    }
    if (command == DELETE) {
        if (execute_delete(connection, client_fd) != 0) {
            return 1;
        }
        write_to_socket(client_fd, "OK\n", 3);
    }
    epoll_ctl(epfd_, EPOLL_CTL_DEL, client_fd, NULL);
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
    return 0;
}

int execute_get(ConnectState* connection, int client_fd) {
    int len = strlen(temp_dir_) + strlen(connection->server_filename) + 2;
	char file_path[len];
	memset(file_path , 0, len);
	sprintf(file_path, "%s/%s", temp_dir_, connection->server_filename);
    FILE* local_file = fopen(file_path, "r");
    if(!local_file) {
        write_to_socket(client_fd, err_no_such_file, strlen(err_no_such_file));
        exit(1);
    }
    write_to_socket(client_fd, "OK\n", 3);
    size_t fsize = *(size_t*)dictionary_get(server_file_sizes_, connection->server_filename);
	write_to_socket(client_fd, (char*)&fsize, sizeof(size_t));
    size_t w_count = 0;
    size_t header;
    while (w_count < fsize) {
        if ((fsize - w_count) <= 1024 ){
            header = fsize - w_count;
        } else {
            header = 1024;
        }
        char buffer[header + 1];
        fread(buffer, 1, header, local_file);
        write_to_socket(client_fd, buffer, header);
        w_count += header;
    }
    fclose(local_file);
    return 0;
}

int execute_put(ConnectState* connection, int client_fd) {
	int len = strlen(temp_dir_) + strlen(connection->server_filename) + 2;
	char file_path[len];
	memset(file_path , 0, len);
	sprintf(file_path, "%s/%s", temp_dir_, connection->server_filename);
    FILE* read_file = fopen(file_path, "rb");
    FILE* write_file = fopen(file_path, "wb");
    if (!write_file) {
        perror("fopen()");
        return 1;
    }
    size_t buff;
    read_from_socket(client_fd, (char*) &buff, sizeof(size_t));
    size_t read_bytes = 0;
    while (read_bytes < buff + 4) {
        size_t header_size;
        if ((buff + 4 - read_bytes) <= 1024) {
            header_size = (buff + 4 - read_bytes);
        } else {
            header_size = 1024;
        }
        char buffer[1025];
        memset(buffer, 0, 1025);
        ssize_t read_c = read_from_socket(client_fd, buffer, header_size);
        if (read_c == -1) continue;
        fwrite(buffer, 1, read_c, write_file);
        read_bytes += read_c;
         if (read_c == 0) break;
    }
    if (!read_file) {
        vector_push_back(server_files_, connection->server_filename);
    } else {
        fclose(read_file);
    }
    fclose(write_file);
    dictionary_set(server_file_sizes_, connection->server_filename, &buff);
    return 0;
}

int execute_list(ConnectState* connection, int client_fd) {
    size_t size = 0;
    VECTOR_FOR_EACH(server_files_, file, {
        size += strlen(file) + 1;
    });
    if (size) size--;
    write_to_socket(client_fd, (char*) &size, sizeof(size_t));
    VECTOR_FOR_EACH(server_files_, file, {
        write_to_socket(client_fd, file, strlen(file));
        if (_it != _iend-1) {
            write_to_socket(client_fd, "\n", 1);
        }
    });
    return 0;
}

int execute_delete(ConnectState* connection, int client_fd) {
    int len = strlen(temp_dir_) + strlen(connection->server_filename) + 2;
	char file_path[len];
	memset(file_path , 0, len);
	sprintf(file_path, "%s/%s", temp_dir_, connection->server_filename);
    if (remove(file_path) == -1) {
        write_to_socket(client_fd, err_no_such_file, strlen(err_no_such_file));
        exit(1);
    }
    int i = 0;
    VECTOR_FOR_EACH(server_files_, filename, {
        if (strcmp((char *) filename, connection->server_filename) == 0) {
            vector_erase(server_files_, i);
            return 0;
        }
        i++;
    });
    write_to_socket(client_fd, err_no_such_file, strlen(err_no_such_file));
    return 1;
}