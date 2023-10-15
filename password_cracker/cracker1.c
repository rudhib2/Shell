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

// typedef struct craker_t {
//     pthread_t tid;
//     size_t index;
// } cracker_t;


// int start(size_t thread_count) {
//     // TODO your code here, make sure to use thread_count!
//     // Remember to ONLY crack passwords in other threads
//     return 0; // DO NOT change the return code since AG uses it to check if your
//               // program exited normally
// }

// pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// static int recoverd = 0;
static queue *tasks = NULL;
// static cracker_t *crackers = NULL;
static int total = 0;


#include "cracker1.h"
#include "format.h"
#include "utils.h"
#define USERNAME_MAX_LEN 9
#define HASH_MAX_LEN 14
#define PASSWORD_MAX_LEN 32

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int passwordsRecovered = 0;

typedef struct {
    pthread_t threadId;
    size_t threadIndex;
} CrackerThread;

static void* crackPassword(void* data);
static void updateRecoveredCount();
static int isPasswordFound(const char* encryptedPassword, const char* candidatePassword);

void* crackPassword(void* data) {
    CrackerThread* cracker = (CrackerThread*)data;
    char* taskData = NULL;
    char username[USERNAME_MAX_LEN];
    char storedHash[HASH_MAX_LEN];
    char candidatePassword[PASSWORD_MAX_LEN];

    while ((taskData = queue_pull(tasks)) != NULL) {
        sscanf(taskData, "%s %s %s", username, storedHash, candidatePassword);
        v1_print_thread_start(cracker->threadIndex, username);
        double startTime = getThreadCPUTime();
        int prefixLength = getPrefixLength(candidatePassword);
        int hashAttempts = 0;
        int passwordFound = 0;

        if (prefixLength == (int)strlen(candidatePassword)) {
            passwordFound = 1;
        } else {
            int suffixLength = strlen(candidatePassword) - prefixLength;
            struct crypt_data cryptData;
            cryptData.initialized = 0;
            setStringPosition(candidatePassword + prefixLength, 0);
            int totalVariations = 1;
            for (int i = 0; i < suffixLength; ++i) {
                totalVariations *= 26;
            }
            for (int i = 0; i < totalVariations; ++i) {
                ++hashAttempts;
                if (isPasswordFound(crypt_r(candidatePassword, "xx", &cryptData), storedHash)) {
                    updateRecoveredCount();
                    passwordFound = 1;
                    break;
                }
                incrementString(candidatePassword);
            }
        }
        v1_print_thread_result(cracker->threadIndex, username, candidatePassword, 
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
    CrackerThread* threads = malloc(threadCount * sizeof(CrackerThread));
    for (size_t i = 0; i < threadCount; ++i) {
        threads[i].threadIndex = i + 1;
        pthread_create(&(threads[i].threadId), NULL, crackPassword, &(threads[i]));
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
        pthread_join(threads[i].threadId, NULL);
    }
    v1_print_summary(passwordsRecovered, total - passwordsRecovered);
    free(threads);
    queue_destroy(tasks);
    return 0;
}