
/**
 * mad_mad_access_patterns
 * CS 341 - Fall 2023
 */
#include "tree.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

#include <assert.h>


/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

int helper (uint32_t uin, FILE *file, char *ch) {
if (uin == 0) {
return uin;
}

fseek(file, uin, SEEK_SET);
BinaryTreeNode node;

fread(&node, sizeof(BinaryTreeNode), 1, file);
fseek(file, sizeof(BinaryTreeNode) + uin, SEEK_SET);

char node_ch[10];
fread(node_ch, 10, 1, file);
if ((strcmp(ch, node_ch) > 0 && helper(node.right_child, file, ch)) || (strcmp(ch, node_ch) < 0 && helper(node.left_child, file, ch))) {
return 1;
} else if (strcmp(ch, node_ch) == 0) {
        printFound(node_ch, node.count, node.price);
        return 1;
    }
return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
printArgumentUsage();
exit(1);
}

char *name = argv[1];
FILE * file = fopen(name, "r");

if (!file) {
openFail(name);
exit(2);
}

char node[BINTREE_ROOT_NODE_OFFSET];
fread(node, 1, BINTREE_ROOT_NODE_OFFSET, file);

if (strcmp(node, BINTREE_HEADER_STRING) != 0) {
formatFail(name);
exit(2);
}
   
for (int j = 2; j < argc; ++j) {
int flag_ = helper(BINTREE_ROOT_NODE_OFFSET, file, argv[j]);
if (flag_ == 0) {
printNotFound(argv[j]);
}
}

fclose(file);
    return 0;
}

void search_word(FILE *, char *);

/*
    Look up a few nodes in the tree and print the info they contain.
    This version uses fseek() and fread() to access the data.
   
    ./lookup1 <data_file> <word> [<word> ...]
*/