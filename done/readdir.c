/*
 * @file readdir.c
 * @brief Implementation of readdir
 *
 * @author Matteo Rizzo
 */
#include <stdlib.h>
#include <string.h>

#include "disk.h"
#include "fs_api.h"
#include "fs_util.h"

int lab3_readdir(const char *path, char ***out, uint32_t *out_size)
{
    if (!path || !out || !out_size) return -1;

    /* No relative path allowed */
    if (strstr(path, "/") != path) return -1;

    if (strstr(path, ".") || strstr(path, "..") || strstr(path, "~")) return -1;

    struct lab3_inode *inode = find_inode_by_path(path);

    if (!inode) return -1;

    /* File is not allowed */
    if (inode->is_directory == 0) {
        free(inode);

        return -1;
    }

    uint32_t num_children = inode->directory.num_children;
    size_t inode_aligned_size = sizeof(struct lab3_inode);
    struct lab3_inode *child_inode = (struct lab3_inode *)malloc(inode_aligned_size);

    *out = (char **)malloc(sizeof(char *)*num_children);
    *out_size = num_children;

    for (int i = 0; i < num_children; ++i) {
        if (read_from_disk(inode->directory.children_offsets[i], child_inode, inode_aligned_size) < 0) {
            for (int j = 0; j < i; ++j) free(*(*out + j));
            
            free(*out);
            free(child_inode);
            free(inode);
            
            return -1;
        }

        *(*out + i) = (char *)malloc(sizeof(char)*strlen(child_inode->name) + 1);
        strcpy(*(*out + i), child_inode->name);
    }

    free(child_inode);
    free(inode);

    return 0;
}
