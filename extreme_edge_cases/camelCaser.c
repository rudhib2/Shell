/**
 * extreme_edge_cases
 * CS 341 - Fall 2023
 */
#include "camelCaser.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

//to-do:
//split into sentences in camel_caser
//change everything to lowercase
//capitalise first letter after spaces
//remove spaces
//handle memory



void destroy(char **result) {
    for(size_t i = 0; result[i] != NULL; ++i) {
        free(result[i]);
    }
    free(result);
    result = NULL;
}

char* toLower(char* str) {
    for (size_t i = 0; i <= strlen(str); i++) {
        str[i] = tolower(str[i]);
    }
    return str;
}

char* capsAfterSpace(char* str) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (isspace(str[i])) {
            str[i+1] = toupper(str[i+1]);
        }
    }
    return str;
}

char* removeSpaces(char* str) {
    size_t count = 0;
    for (int i = 0; str[i] != '\0'; i++){
        if (isspace(str[i])){
            continue;
        }
        count++;
    }
    char* without_spaces = (char*)calloc(count + 1, sizeof(char*));  
    // char* without_spaces[count+1];
    int k = 0;
    for (size_t i = 0; i <= strlen(str); i++) {
        if (!isspace(str[i])) { 
            // char* to_Add_substring = (char*)calloc(strlen(str) + 1, sizeof(char*)); //
            // strncpy(without_spaces + end, str + start, i - start); 
            without_spaces[k] = str[i];
            k++;
        } else{
            // k++;
            continue;
        }
    }
    return without_spaces;
}

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    if (input_str == NULL) {
        return NULL;
    }
    int num = 0;
    for (size_t i = 0; i < strlen(input_str); ++i) {
        if (ispunct(input_str[i])) {
            num++;
        }
    }
    int end = 0;
    int start = 0;
    int x = 0;
    char **arr = (char **)calloc(num + 1, sizeof(char *));
    if (arr == NULL) {
        return NULL;  
    }
    for (size_t i = 0; i < strlen(input_str); ++i) {
        if (ispunct(input_str[i])) {
            end = i;
            char *arr1 = (char *)calloc((end - start) + 1, sizeof(char*));
            if (arr1 == NULL) {
                return NULL;
            }
            strncpy(arr1, input_str + start, end - start);
            arr1[end - start] = '\0';
            arr[x] = arr1;
            x++;
            if (!isspace(input_str[i+1])){
                start = end+1;
            } else{
                start = end + 2;
            }
        }
    }
    // printf("g");
    // for (int i = 0; i < num + 1; ++i) {
    //     char *tmp = strdup(arr[i]);
    //     if (tmp == NULL) {
    //         return NULL;
    //     }
    //     printf("%s", tmp);
    //     toLower(tmp);
    //     capsAfterSpace(tmp);
    //     removeSpaces(tmp);
    //     arr[i] = tmp;
    //     printf("%s", tmp);
    //     printf("lo");
    //     free(tmp); // Free the duplicated string
    // }
    for (int i = 0; arr[i] != NULL; i++) {
        arr[i] = toLower(arr[i]);
        arr[i] = capsAfterSpace(arr[i]);
        arr[i] = removeSpaces(arr[i]);
    }
    return arr;
}