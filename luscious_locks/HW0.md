# Welcome to Homework 0!

For these questions you'll need the mini course and  "Linux-In-TheBrowser" virtual machine (yes it really does run in a web page using javascript) at -

http://cs-education.github.io/sys/

Let's take a look at some C code (with apologies to a well known song)-
```C
// An array to hold the following bytes. "q" will hold the address of where those bytes are.
// The [] mean set aside some space and copy these bytes into teh array array
char q[] = "Do you wanna build a C99 program?";

// This will be fun if our code has the word 'or' in later...
#define or "go debugging with gdb?"

// sizeof is not the same as strlen. You need to know how to use these correctly, including why you probably want strlen+1

static unsigned int i = sizeof(or) != strlen(or);

// Reading backwards, ptr is a pointer to a character. (It holds the address of the first byte of that string constant)
char* ptr = "lathe"; 

// Print something out
size_t come = fprintf(stdout,"%s door", ptr+2);

// Challenge: Why is the value of away equal to 1?
int away = ! (int) * "";


// Some system programming - ask for some virtual memory

int* shared = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
munmap(shared,sizeof(int*));

// Now clone our process and run other programs
if(!fork()) { execlp("man","man","-3","ftell", (char*)0); perror("failed"); }
if(!fork()) { execlp("make","make", "snowman", (char*)0); execlp("make","make", (char*)0)); }

// Let's get out of it?
exit(0);
```

## So you want to master System Programming? And get a better grade than B?
```C
int main(int argc, char** argv) {
	puts("Great! We have plenty of useful resources for you, but it's up to you to");
	puts(" be an active learner and learn how to solve problems and debug code.");
	puts("Bring your near-completed answers the problems below");
	puts(" to the first lab to show that you've been working on this.");
	printf("A few \"don't knows\" or \"unsure\" is fine for lab 1.\n"); 
	puts("Warning: you and your peers will work hard in this class.");
	puts("This is not CS225; you will be pushed much harder to");
	puts(" work things out on your own.");
	fprintf(stdout,"This homework is a stepping stone to all future assignments.\n");
	char p[] = "So, you will want to clear up any confusions or misconceptions.\n";
	write(1, p, strlen(p) );
	char buffer[1024];
	sprintf(buffer,"For grading purposes, this homework 0 will be graded as part of your lab %d work.\n", 1);
	write(1, buffer, strlen(buffer));
	printf("Press Return to continue\n");
	read(0, buffer, sizeof(buffer));
	return 0;
}
```
## Watch the videos and write up your answers to the following questions

**Important!**

The virtual machine-in-your-browser and the videos you need for HW0 are here:

http://cs-education.github.io/sys/

The coursebook:

http://cs341.cs.illinois.edu/coursebook/index.html

Questions? Comments? Use Ed: (you'll need to accept the sign up link I sent you)
https://edstem.org/

The in-browser virtual machine runs entirely in Javascript and is fastest in Chrome. Note the VM and any code you write is reset when you reload the page, *so copy your code to a separate document.* The post-video challenges (e.g. Haiku poem) are not part of homework 0 but you learn the most by doing (rather than just passively watching) - so we suggest you have some fun with each end-of-video challenge.

HW0 questions are below. Copy your answers into a text document (which the course staff will grade later) because you'll need to submit them later in the course. More information will be in the first lab.

## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".

#include <unistd.h>
int main() {
	write(1, "Hi! My name is Rudhi", 20);
	return 0;
}

### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
   - The triangle should look like this, for n = 3:
   ```C
   *
   **
   ***
   ```

   #include <unistd.h>
	void write_triangle(int n) {
	int i;
	int j;
		for (i = 1; i <= n; i++) {
			for (j = 0; j < i; j++) {
			write(2, "*", 1);  
			}
		write(2, "\n", 1);
		}
             }

### Writing to files
3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
   - Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).

   int main() {
	int file1 = open(“hello_world.txt”, 0_CREAT | 0_TRUNC | 0_RDWR);
	write (file1, "Hi! My name is Rudhi", 20);
	close(file1);
}
	

### Not everything is a system call
4. Take your program from "Writing to files" and replace `write()` with `printf()`.
   - Make sure to print to the file instead of standard out!

   int main() {
	int file1 = open(“hello_world.txt”, 0_CREAT | 0_TRUNC | 0_RDWR);
	dfprintf (2, "Hi! My name is Rudhi", 20);
	close(file1);
}

5. What are some differences between `write()` and `printf()`?

`write()` is a system function for basic data output to file descriptors, without formatting. It needs explicit length and descriptor specification. 
`printf()` is a C library function offering formatted text output to the standard output or a chosen stream. It's higher-level and more versatile for stylized output.


## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte?

A byte is usually 8 bits but not always
2. How many bytes are there in a `char`?

	1 byte
3. How many bytes the following are on your machine?
   - `int`, `double`, `float`, `long`, and `long long`

	int – 4 bytes
	double – 8 bytes
	float – 4 bytes
	long –  8 bytes
	long long – 8 bytes

### Follow the int pointer
4. On a machine with 8 byte integers:
```C
int main(){
    int data[8];
} 
```
If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?

0x7fbd9d50

5. What is `data[3]` equivalent to in C?
   - Hint: what does C convert `data[3]` to before dereferencing the address?

3[data]

### `sizeof` character arrays, incrementing pointers
  
Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
```C
char *ptr = "hello";
*ptr = 'J';
```
It segfaults because we are trying to modify a string literal which is stored in the read-only section of memory.

7. What does `sizeof("Hello\0World")` return?

12
8. What does `strlen("Hello\0World")` return?

5
9. Give an example of X such that `sizeof(X)` is 3.

ab
10. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.

a pointer

## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?

•	argc 
•	looping through argv to find its length

2. What does `argv[0]` represent?

The name of the program itself 
### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?

The stack
### String searching (strings are just char arrays)
4. On a machine where pointers are 8 bytes, and with the following code:
```C
char *ptr = "Hello";
char array[] = "Hello";
```
What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?

sizeof(ptr) – 8 because pointers are 8 bytes on this machine
sizeof(array) – 6 because Hello consists of 5 characters plus the null terminator 

### Lifetime of automatic variables

5. What data structure manages the lifetime of automatic variables?

The stack

## Chapter 4

Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?

It should be put in the heap. We can use functions like malloc(), calloc(), and realloc() to put data in the heap.
2. What are the differences between heap and stack memory?

Heap is managed dynamically, it is for memory allocation that lasts beyond functions and needs manual release. It’s slower to allocate but good for larger data.
Stack is managed automatically and is used for function calls and local variables. Allocation is faster but because of limited space, memory is automatically freed. 

3. Are there other kinds of memory in a process?

Yes, like static memory, code memory, data memory etc.
4. Fill in the blank: "In a good C program, for every malloc, there is a ___".

free
### Heap allocation gotchas
5. What is one reason `malloc` can fail?

when there is insufficient memory in the heap to allocate the requested amount of memory

6. What are some differences between `time()` and `ctime()`?

`time()` is a function that returns the current time in seconds since the epoch.
`ctime()` is a function that takes a time value (usually obtained from `time()`) and converts it into a human-readable string representation of time and date.

7. What is wrong with this code snippet?
```C
free(ptr);
free(ptr);
```

We are trying to free the memory twice. Calling free(ptr) once releases the allocated memory but doing so the second time would result in undefined behavior.

8. What is wrong with this code snippet?
```C
free(ptr);
printf("%s\n", ptr);
```
The memory has been deallocated after we call free(ptr) so in the print statement we are trying to access memory that has already been deallocated.

9. How can one avoid the previous two mistakes? 

We can set the pointer to null using: ptr = NULL; 

### `struct`, `typedef`s, and a linked list
10. Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s).

struct Person {
	char* name;
	int age;
	struct Person* friends[5];
};

typedef struct Person Person_t;


11. Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.

int main() {
Person_t *aS = (Person_t*) malloc(sizeof(Person_t));
Person_t *sM = (Person_t*) malloc(sizeof(Person_t));
aS->name = “Agent Smith”;
sM->name = “Sonny Moore”;
aS->age = 128;
sM->age = 256;
aS -> friends[0] = sM;
sM -> friends[0] = aS;
return 0;
}

### Duplicating strings, memory allocation and deallocation of structures
Create functions to create and destroy a Person (Person's and their names should live on the heap).
12. `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).

Person_t* person_create(char*aname, char*aage) {
printf(“Creating link %s -> %s”, aname, aage);
Person_t* result = (Person_t*)malloc(sizeof(Person_t));
result -> name = strdup(aname);
result -> age = strdup(aage);
return result;
}

13. `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.

void person_destroy(Person_t* p) {
free (p->name);
free (p ->age);
memset (p, 0, sizeof(Person_t));
free(p);
} 

## Chapter 5 

Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?

puts for stdout and gets for stdin

2. Name one issue with `gets()`.

It does not provide a way to limit the number of characters read – this could be a problem when the input provided is larger than the buffer allocated to store it.

### Introducing `sscanf` and friends
3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".

int main() {
    char input[] = "Hello 5 World";
    char str1[20];
    int num;
    char str2[20];
    sscanf(input, "%s %d %s", str1, num, str2);
    return 0;
}


### `getline` is useful
4. What does one need to define before including `getline()`?

#define _GNU_SOURCE

5. Write a C program to print out the content of a file line-by-line using `getline()`.

int main() { 
FILE *file = fopen(“myfile.xyz”, “r”);
char *buffer = NULL;
size_t capacity = 0;
ssize_t res = getline(&buffer, &capacity, &file);
	if (res > 0 && buffer[res-1] == ‘\n’) {
		buffer[res-1] = 0;
	}
free(buffer);
return EXIT_SUCCESS;
}


## C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here

1. What compiler flag is used to generate a debug build?

-g

2. You modify the Makefile to generate debug builds and type `make` again. Explain why this is insufficient to generate a new build.

Updating a Makefile and using `make` might not always create a new build due to how dependencies work or cached data. Sometimes, starting fresh by cleaning and building again is necessary for changes to be effective.

3. Are tabs or spaces used to indent the commands after the rule in a Makefile?

tabs

4. What does `git commit` do? What's a `sha` in the context of git?

The git commit command is used to save changes to the local repository. Commits can be helpful to track the history of a project. In context of git, sha(secure hash algorithm) is a unique identifier associated with a commit or any other object in the git repository.

5. What does `git log` show you?

git log shows information like commit sha(unique identifier for each commit), author and date, commit message, and changes .

6. What does `git status` tell you and how would the contents of `.gitignore` change its output?

git status shows the untracked, staged, and modified files. .gitignore file hides certain files excluding them from git status.

7. What does `git push` do? Why is it not just sufficient to commit with `git commit -m 'fixed all bugs' `?

git push uploads local commits to a remote repository. Committing with git commit only saves locally; pushing ensures changes are visible to collaborators. Commit messages should also be more detailed so that collaborators and anyone using the code in the future should be able to track the changes.

8. What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?

A "non-fast-forward" error in git push means the remote repository has changes that are not shown in the local repository. These can be resolved by fetching remote changes with git pull, merging, and then pushing.

## Optional (Just for fun)
- Convert your a song lyrics into System Programming and C code and share on Ed.
- Find, in your opinion, the best and worst C code on the web and post the link to Ed.
- Write a short C program with a deliberate subtle C bug and post it on Ed to see if others can spot your bug.
- Do you have any cool/disastrous system programming bugs you've heard about? Feel free to share with your peers and the course staff on Ed.
