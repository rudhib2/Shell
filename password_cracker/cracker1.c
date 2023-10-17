//used ChatGPT for initial design and debugging - worked well
/**
 * password_cracker
 * CS 341 - Fall 2023
 */

#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "includes/queue.h"
#include <stdio.h>
#include <string.h>
#include <crypt.h>
#define USERNAME_MAX_LEN 9
#define HASH_MAX_LEN 14
#define PASSWORD_MAX_LEN 32

static queue *tasks = NULL;
static int total = 0;
static void* crackPassword(void* data);
static void updateRecoveredCount();
static int isPasswordFound(const char* encryptedPassword, const char* candidatePassword);
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int passwordsRecovered = 0;

void* crackPassword(void* data) {
    char* taskData = NULL;
    int threadId = (long) data;
    char username[USERNAME_MAX_LEN];
    char storedHash[HASH_MAX_LEN];
    char candidatePassword[PASSWORD_MAX_LEN];
    while ((taskData = queue_pull(tasks)) != NULL) {
        sscanf(taskData, "%s %s %s", username, storedHash, candidatePassword);
        v1_print_thread_start(threadId, username);
        double startTime = getThreadCPUTime();
        double prefixLength = getPrefixLength(candidatePassword);
        int hashAttempts = 0;
        int passwordFound = 0;
        if (prefixLength != (int)strlen(candidatePassword)) {
            struct crypt_data cryptData;
            setStringPosition(candidatePassword + (int)prefixLength, 0);
            int totalVariations = 1;
            // brute force check all ways
            for (int i = 0; i < (int)strlen(candidatePassword) - prefixLength; ++i) {
                totalVariations *= 26;
                if(i == ((int)strlen(candidatePassword) - prefixLength-1)){
                    for (int i = 0; i < totalVariations; ++i) {
                        hashAttempts++;
                        if (isPasswordFound(crypt_r(candidatePassword, "xx", &cryptData), storedHash)) {
                            updateRecoveredCount();
                            passwordFound = 1;
                            break;
                        }
                        incrementString(candidatePassword);
                    }
                }
            }
        } else{
            passwordFound = 0;
        }
         v1_print_thread_result(threadId, username, candidatePassword, 
            hashAttempts, getThreadCPUTime() - startTime, !passwordFound);
            free(taskData);
            taskData = NULL;
    }
    queue_push(tasks, NULL);
    free(taskData);
    return NULL;
}

void updateRecoveredCount() {
    pthread_mutex_lock(&lock);
    ++passwordsRecovered;
    pthread_mutex_unlock(&lock);
}

int isPasswordFound(const char* encryptedPassword, const char* storedHash) {
    return (strcmp(encryptedPassword, storedHash) == 0);
}

int start(size_t threadCount) {
    tasks = queue_create(0);
    pthread_t tid[threadCount];
    for (size_t i = 0; i < threadCount; i++) {
      pthread_create(tid+i, NULL, crackPassword, (void *)(i+1));
    }
    char* line = NULL;
    size_t size = 0;
    while (getline(&line, &size, stdin) != -1) {
        queue_push(tasks, strdup(line));
        ++total;
    }
    queue_push(tasks, NULL);
    free(line);
    for (size_t i = 0; i < threadCount; ++i) {
        pthread_join(tid[i], NULL);
    }
    v1_print_summary(passwordsRecovered, total - passwordsRecovered);
    queue_destroy(tasks);
    return 0;
}