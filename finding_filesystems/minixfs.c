//used ChatGPT for initial design and debugging

/**
 * finding_filesystems
 * CS 341 - Fall 2023
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
    // return 0;
    inode* target_inode = get_inode(fs, path);
    if (target_inode == NULL) {
        errno = ENOENT;
        return -1;
    }
    uint16_t type = target_inode->mode >> RWX_BITS_NUMBER;
    target_inode->mode = (new_permissions & 0777) | (type << RWX_BITS_NUMBER);
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    target_inode->ctim = current_time;
    return 0; 
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    // return -1;
    inode* target_inode = get_inode(fs,path);
    if (target_inode == NULL) {
        errno = ENOENT;
        return -1;
    }
    if (owner != ((uid_t)-1)) {
        target_inode->uid = owner;
    }
    if (group != ((gid_t)-1)) {
        target_inode->gid = group;
    }
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    target_inode->ctim = current_time;
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    // return NULL;
    // Check if the specified file or directory already exists or if the filename is invalid
    inode *existing_node = get_inode(fs, path);

    if (existing_node || !valid_filename(path)) {
        return NULL;
    }
    const char *new_name;
    inode *parent_inode = parent_directory(fs, path, &new_name);
    inode_number new_inode_number = first_unused_inode(fs);
    init_inode(parent_inode, &(fs->inode_root[new_inode_number]));
    minixfs_dirent new_entry;
    new_entry.name = strdup(new_name);
    new_entry.inode_num = new_inode_number;
    int data_block_index = parent_inode->size / sizeof(data_block);
    size_t remaining_space = parent_inode->size % sizeof(data_block);
    data_block_number block_number;
    if (data_block_index < NUM_DIRECT_BLOCKS) {
        block_number = parent_inode->direct[data_block_index];
        make_string_from_dirent(fs->data_root[block_number].data + remaining_space, new_entry);
    } else {
        block_number = parent_inode->indirect;
        data_block_number* indirect_blocks = (data_block_number*)fs->data_root[block_number].data;
        data_block_number indirect_block_number = indirect_blocks[data_block_index - NUM_DIRECT_BLOCKS];
        make_string_from_dirent(fs->data_root[indirect_block_number].data + remaining_space, new_entry);
    }
    free(new_entry.name);
    return fs->inode_root + new_inode_number;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    // if (!strcmp(path, "info")) {
    //     // TODO implement the "info" virtual file here
    // }
    // errno = ENOENT;
    // return -1;
    if (!strcmp(path, "info")) {
        if (count != 0) {
            errno = EINVAL;
            return -1;
        }
        char* data_map = GET_DATA_MAP(fs->meta);
        ssize_t num_used_block = 0;
        for (uint64_t i = 0; i < fs->meta->dblock_count; i++) {
            if (data_map[i] == 1) {
                num_used_block++;
            }
        }
        char* str = block_info_string(num_used_block);
        long str_len = strlen(str);
        if (*off >= str_len) {
            return 0;
        }
        size_t remaining = str_len - *off;
        if (count > remaining) {
            count = remaining;
        }
        memcpy(buf, str + *off, count);
        *off += count;
        return count;
    }
    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    // return -1;

    size_t current_block_index = *off / sizeof(data_block);
    size_t block_offset = *off % sizeof(data_block);
    off_t write_end = *off + count;
    size_t bytes_written = 0;
    inode *target_inode = get_inode(fs, path);
    target_inode->size = *off + count;
    while (*off < write_end) {
        data_block current_data_block;
        if (current_block_index >= NUM_DIRECT_BLOCKS) {
            data_block_number *indirect_blocks = (data_block_number *)(fs->data_root + target_inode->indirect);
            current_data_block = fs->data_root[indirect_blocks[current_block_index - NUM_DIRECT_BLOCKS]];
        } else {
            current_data_block = fs->data_root[target_inode->direct[current_block_index]];
        }
        size_t bytes_to_write = sizeof(data_block) - block_offset;
        bytes_to_write = (write_end - *off) < (off_t)bytes_to_write ? (size_t)(write_end - *off) : bytes_to_write;
        memcpy(current_data_block.data + block_offset, buf + bytes_written, bytes_to_write);
        *off += bytes_to_write;
        bytes_written += bytes_to_write;
        block_offset = 0;
        ++current_block_index;
    }
    clock_gettime(CLOCK_REALTIME, &target_inode->mtim);
    clock_gettime(CLOCK_REALTIME, &target_inode->atim);
    return bytes_written;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    // // 'ere be treasure!
    // return -1;
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path) {
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    }
    inode *node = get_inode(fs, path);
    if (!node) {
        errno = ENOSPC;
        return -1;
    }
    if ((uint64_t)*off >= node->size) {
        return 0;
    }
    size_t byte_read = 0;
    while (*off < (off_t)node->size) {
        size_t block_index = *off / sizeof(data_block);
        size_t block_offset = *off % sizeof(data_block);
        data_block curr_block;
        if (block_index >= NUM_DIRECT_BLOCKS) {
            data_block_number *indirect_blocks = (data_block_number *)(fs->data_root + node->indirect);
            curr_block = fs->data_root[indirect_blocks[block_index - NUM_DIRECT_BLOCKS]];
        } else {
            curr_block = fs->data_root[node->direct[block_index]];
        }
        size_t bytes_remaining = node->size - *off;
        size_t byte_to_read = (bytes_remaining < sizeof(data_block) - block_offset) ? bytes_remaining : sizeof(data_block) - block_offset;
        memcpy(buf + byte_read, curr_block.data, byte_to_read);
        *off += byte_to_read;
        byte_read += byte_to_read;
    }
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    node->atim = current_time;
    return byte_read;
}
