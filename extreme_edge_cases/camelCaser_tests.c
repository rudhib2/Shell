/**
 * extreme_edge_cases
 * CS 341 - Fall 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **))
{
    // TODO: Implement me!
    // Test cases
    const char *input1 = "This is a Test sentence. Anoth22er oNe here! Third       SenTencE.";
    char **output1 = camelCaser(input1);
    for (int i = 0; output1[i] != NULL; ++i) {
        printf("%s\n", output1[i]);
    }
    // Add more test cases if needed
    return 1;
}