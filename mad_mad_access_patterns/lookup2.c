/**
 * mad_mad_access_patterns
 * CS 341 - Fall 2023
 */
#include "tree.h"
#include "utils.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/mman.h>

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

BinaryTreeNode * helper (uint32_t uin, char* ch, char* word) {
if (uin == 0) {
return NULL;
}
BinaryTreeNode* node = (BinaryTreeNode*) (uin + ch);
BinaryTreeNode* node_ch;
if (strcmp(word, node->word) < 0) {
node_ch = helper(node->left_child, ch, word);
if (node_ch != NULL) {
return node_ch;
}
} else if (strcmp(word, node->word) == 0){
return node;
} else if (strcmp(word, node->word) > 0) {
    node_ch = helper(node->right_child, ch, word);
    if (node_ch != NULL) {
      return node_ch;
    }
}
return NULL;
}

int main(int argc, char **argv) {
  if (argc < 3) {
printArgumentUsage();
exit(1);
}

char* name = argv[1];

int file = open(name, O_RDONLY);
if (file < 0) {
openFail(name);
exit(2);
}

struct stat status;
if (fstat(file, &status) != 0) {
openFail(name);
exit(2);
}

char* ch = mmap(NULL, status.st_size, PROT_READ, MAP_PRIVATE, file, 0);
if (ch == (char*)(-1)) {
mmapFail(name);
exit(2);
}

if (strncmp(ch, BINTREE_HEADER_STRING, BINTREE_ROOT_NODE_OFFSET)) {
    formatFail(name);
    exit(2);
  }

int x = 2;
while (x < argc) {
BinaryTreeNode* node_word = helper(BINTREE_ROOT_NODE_OFFSET, ch, argv[x]);
if (!node_word) {
printNotFound(argv[x]);
} else {
printFound(node_word->word, node_word->count, node_word->price);
}
x++;
}

close(file);
return 0;
}
