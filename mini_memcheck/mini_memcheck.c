/**
 * mini_memcheck
 * CS 341 - Fall 2023
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>

meta_data *head = NULL;
size_t total_memory_requested = 0;
size_t total_memory_freed = 0;
size_t invalid_addresses = 0;

void *mini_malloc(size_t request_size, const char *filename, void *instruction) {
    if (request_size == 0) return NULL;

    meta_data *new_node = (meta_data *)malloc(sizeof(meta_data) + request_size);
    if (!new_node) return NULL;

    new_node->request_size = request_size;
    new_node->filename = filename;
    new_node->instruction = instruction;
    new_node->next = head;
    head = new_node;
    total_memory_requested += request_size;

    return (void *)((char *)new_node + sizeof(meta_data));
}

void *mini_calloc(size_t num_elements, size_t element_size, const char *filename, void *instruction) {
    if (num_elements == 0 || element_size == 0) return NULL;

    size_t request_size = num_elements * element_size;
    void *ptr = mini_malloc(request_size, filename, instruction);
    if (!ptr) return NULL;

    memset(ptr, 0, request_size);
    return ptr;
}

void *mini_realloc(void *ptr, size_t request_size, const char *filename, void *instruction) {
    if (ptr == NULL && request_size == 0) return NULL;  // Undefined behavior

    if (ptr == NULL) return mini_malloc(request_size, filename, instruction);
    if (request_size == 0) {
        mini_free(ptr);
        return NULL;
    }

    meta_data *node = (meta_data *)((char *)ptr - sizeof(meta_data));

    // Validate the ptr is in the linked list
    meta_data *current = head;
    while (current && current != node) current = current->next;
    if (!current) {
        invalid_addresses++;
        return NULL;
    }

    size_t old_size = node->request_size;
    meta_data *new_node = (meta_data *)realloc(node, sizeof(meta_data) + request_size);
    if (!new_node) return NULL;

    // Update metadata and global variables
    new_node->request_size = request_size;
    new_node->filename = filename;
    new_node->instruction = instruction;
    if (request_size > old_size) {
        total_memory_requested += (request_size - old_size);
    } else {
        total_memory_freed += (old_size - request_size);
    }

    // Update the linked list to point to the new node in case the node's address changed due to realloc
    if (node != new_node) {
        meta_data *prev = NULL, *curr = head;
        while (curr && curr != node) {
            prev = curr;
            curr = curr->next;
        }
        if (prev) {
            prev->next = new_node;
        } else {
            head = new_node;  // Update head if the node was the first in the list
        }
        new_node->next = curr ? curr->next : NULL;
    }

    return (void *)((char *)new_node + sizeof(meta_data));
}

void mini_free(void *ptr) {
    if (!ptr) return;

    meta_data *node = (meta_data *)((char *)ptr - sizeof(meta_data));

    // Validate the ptr is in the linked list
    meta_data *prev = NULL, *current = head;
    while (current && current != node) {
        prev = current;
        current = current->next;
    }

    if (!current) {
        invalid_addresses++;
        return;
    }

    // Unlink the node from the linked list
    if (prev) {
        prev->next = node->next;
    } else {
        head = node->next;  // Update head if the node was the first in the list
    }

    total_memory_freed += node->request_size;

    // Free the node memory
    free(node);
}
