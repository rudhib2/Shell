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

/**
 * Takes in an sstring and a character (the delimiter), and splits the sstring
 * into a vector of C-strings on the given delimiter.
 * This should be analogous to python3's split function on
 * strings. You can check what the output should be for a given source string,
 * INPUT and a delimiter D, by running `python3 -c 'print("INPUT".split('D'))'`
 * in a shell.
 *
 * Example:
 * sstring_split(cstr_to_sstring("abcdeefg"), 'e'); // == [ "abcd", "", "fg" ]);
 * sstring_split(cstr_to_sstring("This is a sentence."), ' ');
 * // == [ "This", "is", "a", "sentence." ]
 */

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    
    // int count = 0;
    // for (size_t i = 0; i < vector_size(this->vect); ++i){
    //     if (vector_get(this->vect, i) == delimiter) {
    //         count++;
    //     }
    // }

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


int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    return -1;
}


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
