/**
 * perilous_pointers
 * CS 341 - Fall 2023
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);

    int input2 = 132;
    second_step(&input2);

    int input3 = 8942;	
	int *ptr = &input3;
	int **inp_3 = &ptr;
	double_step(inp_3);

    char input4[10];
	for (int i = 0; i < 10; i++){
        input4[i] = 0; 
    }
	int num = 15;
	input4[5] = (char) num;
	strange_step(input4);

    char *input_5 = "abc";
	empty_step(input_5);

    char *input_6 = "uuuuu";
	char *s = input_6;
	two_step(s, input_6);

    char *first = "xyz";
    char *second = first+2;
    char *third = second+2;
    three_step(first, second, third);
    
    char one[] = "hello";
    char two[5];
	two[2] = (char)(one[1] + 8);
    char three[5];
	three[3] = (char)(two[2] + 8);
	step_step_step(one, two, three);

    int b = 17;
	char a = 17;
	it_may_be_odd(&a, b);

    char input_10[] = " ,CS241";
    tok_step(input_10);

    char blue[10];
	for (int i = 0; i < 10; i++){
        blue[i] = 0;
    }
	blue[0] = 1; 
    blue[1] = 20; 
    blue[2] = 0; 
    blue[3] = 0;
	the_end(blue, blue);

    return 0;
}
