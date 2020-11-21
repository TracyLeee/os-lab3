/*
 * @file open.c
 * @brief Implementation of open
 *
 * @author Matteo Rizzo
 */
#include <stdlib.h>
#include <string.h>

#include "fs_api.h"
#include "fs_util.h"
#include "open_file_table.h"

int lab3_open(const char *path)
{
    if (!path) return -1;

    /* No relative path allowed */
    if (strstr(path, "/") != path) return -1;

    if (strstr(path, ".") || strstr(path, "..") || strstr(path, "~")) return -1;

    /* Attempt to find the inode */
    struct lab3_inode *inode = find_inode_by_path(path);

    if (!inode) return -1;

    /* Opening directory is prohibited */
    if (inode->is_directory > 0) {
        free(inode);
        
        return -1;
    }

    /* Search for a free slot in open file table */
    int fd = -1;

    for (int i = 0; i < MAX_OPEN_FILES; ++i) {
        if (open_file_table[i].inode) {
            /* No multi-opening allowed */
            if (open_file_table[i].inode->id == inode->id) {
                free(inode);

                return -1;
            }
        } else {
            if (fd < 0) fd = i;
        }
    }

    if (fd < 0) {
        free(inode);

        return -1;
    } else {
        open_file_table[fd].inode = inode;
        open_file_table[fd].seek_offset = 0;

        return fd;
    }
}
