//used ChatGPT for initial design and debugging - it works well
/**
 * malloc
 * CS 341 - Fall 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

typedef struct _metadata_entry_t {
    void *ptr;
    size_t size;
    int free;
    struct _metadata_entry_t *next;
    struct _metadata_entry_t *prev;
} metadata_entry_t;

static metadata_entry_t *head = NULL;
static size_t total, tot_sbrk= 0;

// /**
//  * Allocate space for array in memory
//  *
//  * Allocates a block of memory for an array of num elements, each of them size
//  * bytes long, and initializes all its bits to zero. The effective result is
//  * the allocation of an zero-initialized memory block of (num * size) bytes.
//  *
//  * @param num
//  *    Number of elements to be allocated.
//  * @param size
//  *    Size of elements.
//  *
//  * @return
//  *    A pointer to the memory block allocated by the function.
//  *
//  *    The type of this pointer is always void*, which can be cast to the
//  *    desired type of data pointer in order to be dereferenceable.
//  *
//  *    If the function failed to allocate the requested block of memory, a
//  *    NULL pointer is returned.
//  *
//  * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
//  */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    size_t total = num * size;
    void *result = malloc(total);
    if (!result) return NULL;
    memset(result, 0, total);
    return result;
}

// /**
//  * Allocate memory block
//  *
//  * Allocates a block of size bytes of memory, returning a pointer to the
//  * beginning of the block.  The content of the newly allocated block of
//  * memory is not initialized, remaining with indeterminate values.
//  *
//  * @param size
//  *    Size of the memory block, in bytes.
//  *
//  * @return
//  *    On success, a pointer to the memory block allocated by the function.
//  *
//  *    The type of this pointer is always void*, which can be cast to the
//  *    desired type of data pointer in order to be dereferenceable.
//  *
//  *    If the function failed to allocate the requested block of memory,
//  *    a null pointer is returned.
//  *
//  * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
//  */

void *malloc(size_t size) {
    // implement malloc!


    metadata_entry_t *p = head;
    metadata_entry_t *chosen = NULL;

    while (p != NULL && tot_sbrk-total >= size) {
        if (p->free && p->size >= size) {
            chosen = p;
            if (chosen->size >= 2 * size && (chosen->size - size) >= 1024) {
                metadata_entry_t *neww = chosen->ptr + size;
                neww->ptr = (neww + 1);
                neww->free = 1;
                neww->size = chosen->size - size - sizeof(metadata_entry_t);
                neww->next = chosen;
                if (chosen->prev) {
                    chosen->prev->next = neww;
                } else {
                    head = neww;
                }
                neww->prev = chosen->prev;
                chosen->size = size;
                chosen->prev = neww;
                total += sizeof(metadata_entry_t);
            }
            break;
        }
        p = p->next;
    }

//     if(chosen) {
//         chosen->free = 0;
//         return chosen->ptr;
//     }
//     chosen = sbrk(sizeof(metadata_entry_t));
//     chosen->ptr = sbrk(size);
//     if(chosen->ptr == (void*)-1) {
//         return NULL;
//     } 
//     chosen->size = size;
//     chosen->free = 0; 
//     chosen->next = head;
//     head = chosen;
//     return chosen->ptr;
    
    if (chosen != NULL) {
        chosen->free = 0;
        total += chosen->size;
    } else {
        if (head && head->free) {
            if (sbrk(size - head->size) == (void *)-1)
                return NULL;
            tot_sbrk += size - head->size;
            head->size = size;
            head->free = 0;
            chosen = head;
            total += head->size;
        } else {
            chosen = sbrk(sizeof(metadata_entry_t) + size);
            if (chosen == (void *)-1)
                return NULL;
            chosen->ptr = chosen + 1;
            chosen->size = size;
            chosen->free = 0;
            chosen->next = head;
            if (head) {
                chosen->prev = head->prev;
                head->prev = chosen;
            } else {
                chosen->prev = NULL;
            }
            head = chosen;
            tot_sbrk += sizeof(metadata_entry_t) + size;
            total += sizeof(metadata_entry_t) + size;
        }
    }
    return chosen->ptr;
}

// /**
//  * Deallocate space in memory
//  *
//  * A block of memory previously allocated using a call to malloc(),
//  * calloc() or realloc() is deallocated, making it available again for
//  * further allocations.
//  *
//  * Notice that this function leaves the value of ptr unchanged, hence
//  * it still points to the same (now invalid) location, and not to the
//  * null pointer.
//  *
//  * @param ptr
//  *    Pointer to a memory block previously allocated with malloc(),
//  *    calloc() or realloc() to be deallocated.  If a null pointer is
//  *    passed as argument, no action occurs.
//  */

void free(void *ptr) {
    if (!ptr) return;
    metadata_entry_t *p = ((metadata_entry_t*)ptr) - 1;
    while (p) {
        if (p->ptr == ptr) {
            p->free = 1;
            total -= p->size;

            // Coalesce previous and next blocks if they are free
            if (p->prev && p->prev->free == 1) {
                p->size += p->prev->size + sizeof(metadata_entry_t);
                p->prev = p->prev->prev;
                if (p->prev) {
                    p->prev->next = p;
                } else {
                    head = p;
                }
                total -= sizeof(metadata_entry_t);
            }
            if (p->next && p->next->free == 1) {
                p->next->size += p->size + sizeof(metadata_entry_t);
                p->next->prev = p->prev;
                if (p->prev) {
                    p->prev->next = p->next;
                } else {
                    head = p->next;
                }
                total -= sizeof(metadata_entry_t);
            }
            return;
        }
        p = p->next;
    }
}

// /**
//  * Reallocate memory block
//  *
//  * The size of the memory block pointed to by the ptr parameter is changed
//  * to the size bytes, expanding or reducing the amount of memory available
//  * in the block.
//  *
//  * The function may move the memory block to a new location, in which case
//  * the new location is returned. The content of the memory block is preserved
//  * up to the lesser of the new and old sizes, even if the block is moved. If
//  * the new size is larger, the value of the newly allocated portion is
//  * indeterminate.
//  *
//  * In case that ptr is NULL, the function behaves exactly as malloc, assigning
//  * a new block of size bytes and returning a pointer to the beginning of it.
//  *
//  * In case that the size is 0, the memory previously allocated in ptr is
//  * deallocated as if a call to free was made, and a NULL pointer is returned.
//  *
//  * @param ptr
//  *    Pointer to a memory block previously allocated with malloc(), calloc()
//  *    or realloc() to be reallocated.
//  *
//  *    If this is NULL, a new block is allocated and a pointer to it is
//  *    returned by the function.
//  *
//  * @param size
//  *    New size for the memory block, in bytes.
//  *
//  *    If it is 0 and ptr points to an existing block of memory, the memory
//  *    block pointed by ptr is deallocated and a NULL pointer is returned.
//  *
//  * @return
//  *    A pointer to the reallocated memory block, which may be either the
//  *    same as the ptr argument or a new location.
//  *
//  *    The type of this pointer is void*, which can be cast to the desired
//  *    type of data pointer in order to be dereferenceable.
//  *
//  *    If the function failed to allocate the requested block of memory,
//  *    a NULL pointer is returned, and the memory block pointed to by
//  *    argument ptr is left unchanged.
//  *
//  * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
//  */

void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if (!ptr) {
        return malloc(size);
    }
    metadata_entry_t *entry = ((metadata_entry_t *)ptr) - 1;
    assert(entry->ptr == ptr);
    assert(entry->free == 0);
    if (!size) {
        free(ptr);
        return NULL;
    }
    if (entry->size >= size) {
        return ptr;
    } else if (entry->prev && entry->prev->free && entry->size + entry->prev->size + sizeof(metadata_entry_t) >= size) {
        total += entry->prev->size;
        entry->size += entry->prev->size + sizeof(metadata_entry_t);
        entry->prev = entry->prev->prev;
        if (entry->prev) {
            entry->prev->next = entry;
        } else {
            head = entry;
        }
        return entry->ptr;
    }
    void *new_ptr = malloc(size);
    if (!new_ptr) return NULL;
    memcpy(new_ptr, ptr, entry->size);
    free(ptr);
    return new_ptr;
}