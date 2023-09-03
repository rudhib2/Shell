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
    const char *input1 = "This is a T3333est sentence... Anoth22er oNe here! Third       SenTencE.";
    char **output1 = camelCaser(input1);
    for (int i = 0; output1[i] != NULL; ++i) {
        printf("%s\n", output1[i]);
    }
    destroy(output1);

    const char *input2 = "I am a test58484 case938 for numbers3768989.";
    char **output2 = camelCaser(input2);
    for (int i = 0; output2[i] != NULL; ++i) { 
        printf("%s\n", output2[i]);
    }
    destroy(output2);

    const char *input3 = "hello I am                      a test case for multiple spaces.";
    char **output3 = camelCaser(input3);
    for (int i = 0; output3[i] != NULL; ++i) { 
        printf("%s\n", output3[i]);
    }
    destroy(output3);


    const char *input4 = "Hi this is a test case for different punctuations!.,.";
    char **output4 = camelCaser(input4);
    for (int i = 0; output4[i] != NULL; ++i) { 
        printf("%s\n", output4[i]);
    }
    destroy(output4);

    const char *input5 = "Hi this is a test case for multiple punctuations...";
    char **output5 = camelCaser(input5);
    for (int i = 0; output5[i] != NULL; ++i) { 
        printf("%s\n", output5[i]);
    }
    destroy(output5);

    const char *input6 = "659806698.";
    char **output6 = camelCaser(input6);
    for (int i = 0; output6[i] != NULL; ++i) { 
        printf("%s\n", output6[i]);
    }
    destroy(output6);

    const char *input7 = "UppEr CaSes ANd LowerCASES Mixed.";
    char **output7 = camelCaser(input7);
    for (int i = 0; output7[i] != NULL; ++i) { 
        printf("%s\n", output7[i]);
    }
    

    const char *input8 = "This a the first sentence. Hello I am the second sentence. Third sentence here!";
    char **output8 = camelCaser(input8);
    for (int i = 0; output8[i] != NULL; ++i) { 
        printf("%s\n", output8[i]);
    }
    destroy(output8);


    const char *input9 = "r!";
    char **output9 = camelCaser(input9);
    for (int i = 0; output9[i] != NULL; ++i) { 
        printf("%s\n", output9[i]);
    }
    destroy(output9);

    const char *input10 = "R!";
    char **output10 = camelCaser(input10);
    for (int i = 0; output10[i] != NULL; ++i) { 
        printf("%s\n", output10[i]);
    }
    destroy(output10);

    // Add more test cases if needed
    return 1;

}