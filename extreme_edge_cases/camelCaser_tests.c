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
    int success = 1;

    // Test cases
    const char *input1 = "This is a Test sentence. Anoth22er oNe here! Third       SenTencE.";
    char **output1 = camelCaser(input1);
    if (output1)
    {
        printf("Test 1:\n");
        for (int i = 0; output1[i] != NULL; i++)
        {
            printf("%s\n", output1[i]);
            // Add more checks if needed
        }
        destroy(output1);
    }
    else
    {
        printf("Test 1 failed.\n");
        success = 0;
    }

    // Add more test cases if needed

    return success;
    return 1;
}