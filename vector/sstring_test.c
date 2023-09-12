/**
 * vector
 * CS 341 - Fall 2023
 */
#include "sstring.h"
#include <string.h>

int main(int argc, char *argv[]) {
    // TODO create some tests
    sstring * ss = cstr_to_sstring("my name is rudhi ");
    char * cc = sstring_to_cstr(ss);
    printf("the string is: %s\n", cc);

    // str->vect = char_vector_create();
    // for (int i = 0; i < 5; ++i) {
    //     int* r =vector_get(str->vect, i);
    //     printf("The element is %d\n", *r);
    // }

    sstring *str1 = cstr_to_sstring("abc");
    sstring* str2 = cstr_to_sstring("def");
    int len = sstring_append(str1, str2); // len == 6
    char * res = sstring_to_cstr(str1);
    printf("string is %s\n:", res); // == "abcdef"
    printf ("len is %d\n:", len);


    sstring *slice_me = cstr_to_sstring("1234567890");
    
     // == "345" (don't forget about the null byte!)

    char * toret = sstring_slice(slice_me, 2, 5);
    printf("the string is: %s\n", toret);

    vector * toprint = sstring_split(cstr_to_sstring("abcdeefg"), 'e');
    for (size_t i = 0; i < 3; ++i) {
        printf("%s\n", (char*) vector_get(toprint, i));
    }
   
  // == [ "This", "is", "a", "sentence." ]

  // printf("here hsbhs");
  
    return 0;
}
