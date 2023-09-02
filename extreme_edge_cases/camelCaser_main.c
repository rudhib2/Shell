/**
 * extreme_edge_cases
 * CS 341 - Fall 2023
 */
#include <stdio.h>
#include <stdlib.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int main() {
    // char text[] = "Heicsi dfur vkb. uv wsbvuv v bhv. dihi.";
    // toLower(text);
    // capsAfterSpace(text);
    // Feel free to add more test cases of your own!
    if (test_camelCaser(&camel_caser, &destroy)) {
        printf("SUCCESS\n");
    } else {
        printf("FAILED\n");
    }
}
