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
    // const char *input1 = "This is a T3333est sentence... Anoth22er oNe here! Third       SenTencE.";
    // char **output1 = camelCaser(input1);
    // for (int i = 0; output1[i] != NULL; ++i) {
    //     printf("%s\n", output1[i]);
    // }
    // destroy(output1);

    // const char *input2 = "I am a test58484 case938 for numbers3768989.";
    // char **output2 = camelCaser(input2);
    // for (int i = 0; output2[i] != NULL; ++i) { 
    //     printf("%s\n", output2[i]);
    // }
    // destroy(output2);

    // const char *input3 = "hello I am                      a test case for multiple spaces.";
    // char **output3 = camelCaser(input3);
    // for (int i = 0; output3[i] != NULL; ++i) { 
    //     printf("%s\n", output3[i]);
    // }
    // destroy(output3);


    // const char *input4 = "Hi this is a test case for different punctuations!.,.";
    // char **output4 = camelCaser(input4);
    // for (int i = 0; output4[i] != NULL; ++i) { 
    //     printf("%s\n", output4[i]);
    // }
    // destroy(output4);

    // const char *input5 = "Hi this is a test case for multiple punctuations...";
    // char **output5 = camelCaser(input5);
    // for (int i = 0; output5[i] != NULL; ++i) { 
    //     printf("%s\n", output5[i]);
    // }
    // destroy(output5);

    // const char *input6 = "659806698.";
    // char **output6 = camelCaser(input6);
    // for (int i = 0; output6[i] != NULL; ++i) { 
    //     printf("%s\n", output6[i]);
    // }
    // destroy(output6);

    // const char *input7 = "UppEr CaSes ANd LowerCASES Mixed.";
    // char **output7 = camelCaser(input7);
    // for (int i = 0; output7[i] != NULL; ++i) { 
    //     printf("%s\n", output7[i]);
    // }
    

    // const char *input8 = "This a the first sentence. Hello I am the second sentence. Third sentence here!";
    // char **output8 = camelCaser(input8);
    // for (int i = 0; output8[i] != NULL; ++i) { 
    //     printf("%s\n", output8[i]);
    // }
    // destroy(output8);


    // const char *input9 = "r!";
    // char **output9 = camelCaser(input9);
    // for (int i = 0; output9[i] != NULL; ++i) { 
    //     printf("%s\n", output9[i]);
    // }
    // destroy(output9);

    // const char *input10 = "R!";
    // char **output10 = camelCaser(input10);
    // for (int i = 0; output10[i] != NULL; ++i) { 
    //     printf("%s\n", output10[i]);
    // }
    // destroy(output10);

    // const char *input11 = "";
    // char **output11 = camelCaser(input11);
    // for (int i = 0; output11[i] != NULL; ++i) { 
    //     printf("%s\n", output11[i]);
    // }
    // destroy(output11);

    // const char *input12 = NULL;
    // char **output12 = camelCaser(input12);
    // if(output12 == NULL) {
    //     printf("null works\n");
    // }
    // destroy(output12);


    const char *input1 = "This is a T3333est sentence... Anoth22er oNe here! Third       SenTencE.";
    char **output1 = camelCaser(input1);
    if (!output1) {
        return 0;
    }
    if (strcmp(output1[0], "thisIsAT3333estSentence")) {
        return 0;
    }
    if (strcmp(output1[1], "")) {
        return 0;
    }
    if (strcmp(output1[2], "")) {
        return 0;
    }
    if (strcmp(output1[3], "anoth22erOneHere")) {
        return 0;
    }
    if (strcmp(output1[4], "thirdSentence")) {
        return 0;
    }
    if (output1[5] != NULL) {
        return 0;
    }
    printf("test one is complete\n");
    destroy(output1);
    

    const char *input2 = "I am a test58484 case938 for numbers3768989.";
    char **output2 = camelCaser(input2);
    if (!output2) {
        return 0;
    }
    if (strcmp(output2[0], "iAmATest58484Case938ForNumbers3768989")) {
        return 0;
    }
    if (output2[1] != NULL) {
        return 0;
    }
    printf("test two is complete\n");
    destroy(output2);

    const char *input3 = "hello I am                      a test case for multiple spaces.";
    char **output3 = camelCaser(input3);
    if (!output3) {
        return 0;
    }
    if (strcmp(output3[0], "helloIAmATestCaseForMultipleSpaces")) {
        return 0;
    }
    if (output3[1] != NULL) {
        return 0;
    }
    printf("test three is complete\n");
    destroy(output3);

    const char *input4 = "Hi this is a test case for different punctuations!.,.";
    char **output4 = camelCaser(input4);
    if (!output4) {
        return 0;
    }
    if (strcmp(output4[0], "hiThisIsATestCaseForDifferentPunctuations")) {
        return 0;
    }
    if (strcmp(output4[1], "")) {
        return 0;
    }
    if (strcmp(output4[2], "")) {
        return 0;
    }
    if (strcmp(output4[3], "")) {
        return 0;
    }
    if (output4[4] != NULL) {
        return 0;
    }
    printf("test four is complete\n");
    destroy(output4);

    const char *input5 = "Hi this is a test case for multiple punctuations...";
    char **output5 = camelCaser(input5);
    if (!output5) {
        return 0;
    }
    if (strcmp(output5[0], "hiThisIsATestCaseForMultiplePunctuations")) {
        return 0;
    }
    if (strcmp(output5[1], "")) {
        return 0;
    }
    if (strcmp(output5[2], "")) {
        return 0;
    }
    if (output5[3] != NULL) {
        return 0;
    }
    printf("test five is complete\n");
    destroy(output5);

    const char *input6 = "659806698.";
    char **output6 = camelCaser(input6);
    if (strcmp(output6[0], "659806698")) {
        return 0;
    }
    if (output6[1] != NULL) {
        return 0;
    }
    printf("test six is complete\n");
    destroy(output6);

    const char *input7 = "UppEr CaSes ANd LowerCASES Mixed.";
    char **output7 = camelCaser(input7);
    if (strcmp(output7[0], "upperCasesAndLowercasesMixed")) {
        return 0;
    }
    if (output7[1] != NULL) {
        return 0;
    }
    printf("test seven is complete\n");
    destroy(output7);
    

    const char *input8 = "This a the first sentence. Hello I am the second sentence. Third sentence here!";
    char **output8 = camelCaser(input8);
    if (strcmp(output8[0], "thisATheFirstSentence")) {
        return 0;
    }
    if (strcmp(output8[1], "helloIAmTheSecondSentence")) {
        return 0;
    }
     if (strcmp(output8[2], "thirdSentenceHere")) {
        return 0;
    }
    if (output8[3] != NULL) {
        return 0;
    }
    printf("test eight is complete\n");
    destroy(output8);


    const char *input9 = "r!";
    char **output9 = camelCaser(input9);
    if (strcmp(output9[0], "r")) {
        return 0;
    }
    if (output9[1] != NULL) {
        return 0;
    }
    printf("test nine is complete\n");
    destroy(output9);

    const char *input10 = "R!";
    char **output10 = camelCaser(input10);
    if (strcmp(output10[0], "r")) {
        return 0;
    }
    if (output10[1] != NULL) {
        return 0;
    }
    printf("test ten is complete\n");
    destroy(output10);

    // const char *input11 = "";
    // char **output11 = camelCaser(input11);
    // if (strcmp(output11[0], "")) {
    //     return 0;
    // }
    //  printf("test eleven is complete\n");
    // destroy(output11);

    // const char *input11 = "";
    // char **output11 = camelCaser(input11);
    // for (int i = 0; output11[i] != NULL; ++i) { 
    //     printf("%s\n", output11[i]);
    // }
    // destroy(output11);

    const char *input12 = NULL;
    char **output12 = camelCaser(input12);
    if(output12 != NULL) {
        return 0;
    }
    printf("test twelve is complete\n");
    destroy(output12);


    return 1;
}