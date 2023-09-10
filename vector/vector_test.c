/**
 * vector
 * CS 341 - Fall 2023
 */
#include <stdio.h>
#include "vector.h"
int main(int argc, char *argv[]) {
    // Write your test cases here

    vector *test = int_vector_create();
    int x = 1;
    int y = 2;
    int z = 3;
    if(test){
        printf("vector exists\n");
    }
    // //int *y = x;
    
    vector_push_back(test, &x);
    vector_push_back(test, &y);
    vector_push_back(test, &z);
    // printf("jjo\n");
    // for (int i = 0; i < 3; ++i) {
    //     int* r =vector_get(test,i);
    //     printf("%d\n", *r);
    // }
    
    vector_pop_back(test);
    printf("size after pop back is %zu\n", vector_size(test));
    for (int i = 0; i < 2; ++i) {
        int* r =vector_get(test,i);
        printf("%d\n", *r);
    }

    int c = 4;
    int *d = &c;

    vector_insert(test, 1, d);
    for (int i = 0; i < 3; ++i) {
        int* r =vector_get(test,i);
        printf("%d\n", *r);
    }

    size_t s = vector_size(test);
    printf("Size of test is %zu\n", s);

    size_t cap = vector_capacity(test);
    printf("Capacity of test is %zu\n", cap);

    bool bo = vector_empty(test);
    printf("Answer for isempty is: %d\n", bo);

    vector_reserve(test, 10);
    size_t capi = vector_capacity(test);
    printf("Capacity after reserving to 10 is: %zu\n", capi);

    vector_erase(test, 0);
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

    vector *test2 = int_vector_create();
    int x2 = 54;
    int y2 = 28;
    int z2 = 39;
    int a2 = 67;
    int e2 = 98;
    int f2 = 101;
    int j2 = 105;
    
    vector_push_back(test2, &x2);
    vector_push_back(test2, &y2);
    vector_push_back(test2, &z2);
    vector_push_back(test2, &a2);
    vector_push_back(test2, &e2);
    vector_push_back(test2, &f2);
    vector_push_back(test2, &j2);

    vector_resize(test2, 5);

    for (int i = 0; i < 5; ++i) {
        int* r =vector_get(test2,i);
        printf("The elements of test2 are: %d\n", *r);
    }

    size_t size = vector_size(test2);
    printf("Size after resize smaller is:%zu\n", size);

    size_t cap2 = vector_capacity(test2);
    printf("Capacity of test2 is:%zu\n", cap2);

    vector_resize(test2, 10);
    for (int i = 0; i < 10; ++i) {
        int* r =vector_get(test2,i);
        printf("The elements of test2 are: %d\n", *r);
    }

    size_t cap3 = vector_capacity(test2);
    printf("Capacity of test2 is:%zu\n", cap3);

    vector_erase(test2, 3);
    for (int i = 0; i < 9; ++i) {
        int* r =vector_get(test2,i);
        printf("The element is %d\n", *r);
    }

    vector_clear(test2);
    size_t size2 = vector_size(test2);
    printf("Size after clear is:%zu\n", size2);

    vector_reserve(test2, 10000000000000);
    size_t cap4 = vector_capacity(test2);
    printf("Capacity should be 10000000000000:%zu\n", cap4);

    vector_reserve(test2, 2);
    size_t cap5 = vector_capacity(test2);
    printf("Capacity should be 2:%zu\n", cap5);

    vector_reserve(test2, 10000000000000);
    size_t cap6 = vector_capacity(test2);
    printf("Capacity should be 2:%zu\n", cap6);


    vector *test4 = int_vector_create();
    int x3 = 20;
    int y3 = 21;
    int z3 = 22;
    int x4 = 23;
    int y4 = 24;
    int z4 = 25;
    int z5 = 5;
    
    vector_push_back(test4, &x3);
    vector_push_back(test4, &y3);
    vector_push_back(test4, &z3);
    vector_push_back(test4, &x4);
    vector_push_back(test4, &y4);
    vector_push_back(test4, &z4);

    vector_insert(test4, 3, &z5);

    for (int i = 0; i < 7; ++i) {
        int* r =vector_get(test4,i);
        printf("The element is %d\n", *r);
    }

}
