/**
 * vector
 * CS 341 - Fall 2023
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    vector *vect;
};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here

    sstring * result = calloc(1, sizeof(sstring));
    result->vect = char_vector_create();

    for (size_t i = 0; i < strlen(input); ++i) {
        vector_push_back(result->vect, (void *) &input[i]);
    }
    return result;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here

    char * result = calloc(vector_size(input->vect) + 1,sizeof(char *));
    for (size_t i = 0; i < vector_size(input->vect); ++i) {
        result[i] = * (char*) vector_get(input->vect, i);
    }
    result[vector_size(input->vect)] = '\0';
    return result;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    
    // sstring * result = calloc(1, sizeof(sstring));
    // result->vect = char_vector_create();

    // for (size_t i = 0; i < vector_size(this->vect); ++i) {
    //     vector_push_back(result->vect, (void *) &this[i]);
    // }

    for (size_t i = 0; i < vector_size(addition->vect); ++i) {
        vector_push_back(this->vect, vector_get(addition->vect, i));
    }
    return (int) vector_size(this->vect);
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here

    vector *vectorr = string_vector_create();
    sstring *string = calloc(1, sizeof(sstring));
    string->vect = char_vector_create();
    // int start = 0;
    // int end = 0;
    // int tmp = 0;
    
    // char* ending = "\0";
    // vector_insert(this, vector_size(this->vect), (void*) '\0');
    for (size_t i = 0; i < vector_size(this->vect); ++i) {
        // start = end;
        if(* (char *)vector_get(this->vect, i) != delimiter) {
            vector_push_back(string->vect, vector_get(this->vect, i));
        } else {
            vector_push_back(vectorr, sstring_to_cstr(string));
            vector_clear(string->vect);
        }
        // char * topush = sstring_slice(this, start, end+1);
    }
    vector_push_back(vectorr, sstring_to_cstr(string));
    return vectorr;
}

/**
 * This function should substitute one occurrence of target in this after offset
 * bytes with
 * substitution. If there are no occurrences of target after offset bytes,
 * return -1.
 * Otherwise, return 0.
 *
 * You can assume that `offset` is always less than the length of the data in
 * `this`.
 *
 * Example:
 *
 * sstring *replace_me = cstr_to_sstring("This is a {} day, {}!");
 * sstring_substitute(replace_me, 18, "{}", "friend");
 * sstring_to_cstr(replace_me); // == "This is a {} day, friend!"
 * sstring_substitute(replace_me, 0, "{}", "good");
 * sstring_to_cstr(replace_me); // == "This is a good day, friend!"
 */


    // your code goes here

    // //got through all idxes from offset + 1 to size - 1 (if offset + 1 <= size - 1)
    // //match first letter of target
    // //match second letter
    // //match all letters of target
    // //store indicies of the target match
    // //remove target and insert substitution 
    // //if no occurence return -1 
    // // otherwise return 0 
int sstring_substitute(struct sstring* this, size_t offset, char* target, char* substitution) {
    int x = -1;
    if (offset <= vector_size(this->vect) - 1) {
        for (size_t i = offset; i < vector_size(this->vect); ++i) {
            
            for (size_t j = 0; j < strlen(target); ++j) {
                // printf("%c \n", *(char*)vector_get(this->vect, i+j));
                if(*(char*)vector_get(this->vect, i+j) != target[j]) { 
                    // printf("%c \n", *(char*)vector_get(this->vect, i+j));
                    x = 1;
                    break;
                } else {
                    x = -1;
                }
            }
            if (x == -1) {
                
                size_t pos = i;
                for (size_t a = 0; a < strlen(target); ++a) {
                    
                    vector_erase(this->vect, pos);
                }
                
                for (size_t a = 0; a < strlen(substitution); ++a) {
                    vector_insert(this->vect, i+a , &substitution[a]);
                    
                }   
                return 0;
            }
        }
    }
    return -1;
}
    
// int sstring_substitute(sstring *this, size_t offset, char *target,
//                        char *substitution) {
//     char *first_occurence = strstr(sstring_to_cstr(this) + offset, target);
//     if (first_occurence == NULL) {
//         return -1;
//     } else {
//         // char *temp = calloc((strlen(this) + strlen(substitution) - strlen(target) + 1), sizeof(char *));
//         // strcpy(first_occurence, substitution);
//         size_t len1 = strlen(target);
//         size_t len2 = strlen(substitution);
//         if (len1 != len2) {
//             memmove(first_occurence + len2, first_occurence + len1, strlen(first_occurence + len1) + 1);
//         }
//         memcpy(first_occurence, substitution, len2);
//         return 0;
//     }
// }


// int sstring_substitute(struct sstring* this, size_t offset, char* target, char* substitution) {
//     printf("here hsbhs");
//     char* first_occurrence = strstr(sstring_to_cstr(this) + offset, target);
//     if (first_occurrence == NULL) {
//         return -1; 
//     } else {
//         size_t len1 = strlen(target);
//         size_t len2 = strlen(substitution);

//         
//         size_t new_len = strlen(sstring_to_cstr(this)) - len1 + len2;

//         
//         char* new_str = (char*)malloc(new_len + 1);
//         printf("here hsbhs");
//         
//         strncpy(new_str, sstring_to_cstr(this), first_occurrence - sstring_to_cstr(this));
//         printf("here: %s\n", new_str);
//         // new_str[first_occurrence - sstring_to_cstr(this)] = '\0';

//         
//         // strcat(new_str, substitution);

//         
//         // strcat(new_str, first_occurrence + len1);

//         
//         // free(this->vect); // Free the old data
//         // for (size_t i = 0; i < strlen(new_str); ++i){
//         //     vector_set(this->vect, i, new_str[i]) = new_str;

//         // }
        
//         return 0;
//     }
// }



char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    sstring *result = calloc(1, sizeof(sstring));
    result->vect = char_vector_create();

    for (int i = start; i < end; ++i) {
        vector_push_back(result->vect, vector_get(this->vect, i));
    }
    char * toreturn = sstring_to_cstr(result);
    return toreturn;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    vector_destroy(this->vect);
    free(this);
    this = NULL;
}
