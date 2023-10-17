//used ChatGPT for initial design and debugging - worked well

/**
 * password_cracker
 * CS 341 - Fall 2023
 */

#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include <stdio.h>
#include <crypt.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_USERNAME_LENGTH 9
#define MAX_HASH_LENGTH 14
#define MAX_PASSWORD_LENGTH 32

typedef struct UserCredentials {
    char username[MAX_USERNAME_LENGTH + 1];
    char hash[MAX_HASH_LENGTH + 1];
    char target_password[MAX_PASSWORD_LENGTH + 1];
} UserCredentials;

typedef struct CrackerThreadInfo {
    pthread_t thread_id;
    size_t thread_index;
    int hash_count;
} CrackerThreadInfo;

pthread_barrier_t start_barrier;
pthread_barrier_t end_barrier;
static UserCredentials current_user;
static int job_pending;
static char* decrypted_password;
static size_t num_threads;

void* PasswordCrackingTask(void*);

void InitializeCrackers(CrackerThreadInfo* crackers, size_t thread_count) {
    size_t i;
    for (i = 0; i < thread_count; ++i) {
        crackers[i] = (CrackerThreadInfo){0};
        crackers[i].thread_index = i + 1;
        pthread_create(&(crackers[i].thread_id), NULL, PasswordCrackingTask, crackers + i);
    }
}

void ProcessUserInputAndCrackPasswords(CrackerThreadInfo* crackers, size_t thread_count) {
    char* input_line = NULL;
    size_t input_line_size = 0;
    while (getline(&input_line, &input_line_size, stdin) != -1) {
        job_pending = 1;
        decrypted_password = NULL;
        sscanf(input_line, "%s %s %s\n", current_user.username, current_user.hash, current_user.target_password);
        v2_print_start_user(current_user.username);
        pthread_barrier_wait(&start_barrier);
        double start_time = getTime();
        double cpu_start_time = getCPUTime();
        pthread_barrier_wait(&end_barrier);
        int total_hash_count = 0;
        size_t i;
        for (i = 0; i < thread_count; ++i) {
            total_hash_count += crackers[i].hash_count;
            if (i == thread_count - 1){
                int not_found = (decrypted_password == NULL);
                v2_print_summary(current_user.username, decrypted_password, total_hash_count, getTime() - start_time, getCPUTime() - cpu_start_time, not_found);
                free(decrypted_password);
            }
        }
    }
    job_pending = 0;
    pthread_barrier_wait(&start_barrier);
    size_t i;
    for (i = 0; i < thread_count; ++i) {
        pthread_join(crackers[i].thread_id, NULL);
    }
    free(input_line);
}

void* PasswordCrackingTask(void* data) {
    CrackerThreadInfo* data_info = data;
    while (1) {
        pthread_barrier_wait(&start_barrier);
        if (!job_pending) {
            break;
        }
        char* current_password = strdup(current_user.target_password);
        long start_index = 0;
        long count = 0;
        getSubrange(strlen(current_user.target_password) - getPrefixLength(current_user.target_password), num_threads, data_info->thread_index, &start_index, &count);
        setStringPosition(current_password + getPrefixLength(current_user.target_password), start_index);
        v2_print_thread_start(data_info->thread_index, current_user.username, start_index, current_password);
        struct crypt_data crypt_data = {0};
        int status = 1;
        long i;
        for (i = 0; i < count; ++i) {
            if (decrypted_password != NULL) {
                status = 1;
                break;
            }
            data_info->hash_count++;
            if (strcmp(crypt_r(current_password, "xx", &crypt_data), current_user.hash) == 0) {
                status = 0;
                decrypted_password = current_password;
                break;
            }
            incrementString(current_password);
            if(i == count-1 && status){
                free(current_password);
            }
        }
        v2_print_thread_result(data_info->thread_index, data_info->hash_count, status);
        pthread_barrier_wait(&end_barrier);
    }
    return NULL;
}

int start(size_t thread_count) {
    num_threads = thread_count;
    pthread_barrier_init(&start_barrier, NULL, thread_count + 1);
    pthread_barrier_init(&end_barrier, NULL, thread_count + 1);
    CrackerThreadInfo* crackers = malloc(thread_count * sizeof(CrackerThreadInfo));
    InitializeCrackers(crackers, thread_count);
    ProcessUserInputAndCrackPasswords(crackers, thread_count);
    pthread_barrier_destroy(&start_barrier);
    pthread_barrier_destroy(&end_barrier);
    free(crackers);
    return 0;
}