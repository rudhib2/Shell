/**
 * vector
 * CS 341 - Fall 2023
 */
#include <stdio.h>
#include "vector.h"
int main(int argc, char *argv[]) {
    // Write your test cases here
    printf("jjoooffo");
    vector *test = int_vector_create();
    int x = 1;
    int y = 2;
    int z = 3;
    if(test){
        printf("lo");
    }
    // //int *y = x;
    printf("jjooooppppp\n");
    vector_push_back(test, &x);
    vector_push_back(test, &y);
    vector_push_back(test, &z);
    // printf("jjo\n");
    // for (int i = 0; i < 3; ++i) {
    //     int* r =vector_get(test,i);
    //     printf("%d\n", *r);
    // }
    printf("helooo\n");
    vector_pop_back(test);
    printf("size is %zu\n", vector_size(test));
    for (int i = 0; i < 2; ++i) {
        int* r =vector_get(test,i);
        printf("%d\n", *r);
    }

    int c = 4;
    int *d = &c;

    printf("at line 35\n");
    vector_insert(test, 1, d);
    for (int i = 0; i < 3; ++i) {
        int* r =vector_get(test,i);
        printf("%d\n", *r);
    }

    size_t s = vector_size(test);
    printf("%zu\n", s);

    size_t cap = vector_capacity(test);
    printf("%zu\n", cap);

    bool bo = vector_empty(test);
    printf("%d\n", bo);

    vector_reserve(test, 10);
    size_t capi = vector_capacity(test);
    printf("%zu\n", capi);

    vector_erase(test, 1);
    for (int i = 0; i < 2; ++i) {
        int* r =vector_get(test,i);
        printf("The element is %d\n", *r);
    }

    void **end = vector_end(test);
    int *val = *end;
    printf("Value at the end: %d\n", *val);

    void **begin = vector_begin(test);
    int *val2 = *begin;
    printf("Value at the beginning: %d\n", *val2);

    vector_resize(test, 1);
    // for (int i = 0; i < 1; ++i) {
    //     int* r =vector_get(test,i);
    //     printf("The element is %d\n", *r);
    // }
    int* r =vector_get(test,0);
    printf("The element is %d\n", *r);

    // vector_resize(test, 11);
    // for (int i = 0; i < 11; ++i) {
    //     int* r =vector_get(test,i);
    //     printf("The element is %d\n", *r);
    // }

    // return 0;
}
