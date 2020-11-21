/*
 * @file fs_util.c
 * @brief Utility functions used to implement the file system
 *
 * @author Matteo Rizzo, Mark Sutherland
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "disk.h"
#include "fs_util.h"
#include "open_file_table.h"

struct lab3_inode *find_inode_by_path(const char *path)
{
    if (!path) return NULL;
    
    /* Get superblock */
    struct lab3_superblock *superblk = get_disk_superblock();

    if (!superblk) return NULL;

    /* Access the inode of the root directory */
    size_t disk_offset = (superblk->first_dnode_bmap + superblk->num_dnode_bmap_blocks)*DISK_BLK_SIZE;

    free(superblk);

    size_t inode_aligned_size = sizeof(struct lab3_inode);
    struct lab3_inode *inode = (struct lab3_inode *)malloc(inode_aligned_size);

    if (read_from_disk(disk_offset, (void *)inode, inode_aligned_size) < 0) {
        free(inode);
        
        return NULL;
    }

    /* Find the target inode */
    char *path_cpy = (char *)malloc(sizeof(char)*strlen(path) + 1);

    strcpy(path_cpy, path);

    char *component = strtok(path_cpy, "/");
    struct lab3_inode *child_inode = (struct lab3_inode *)malloc(inode_aligned_size);

    while (component) {
        if (inode->is_directory == 0) {
            free(child_inode);
            free(path_cpy);
            free(inode);

            return NULL;
        }

        bool cpnt_found = false;

        for (int i = 0; i < inode->directory.num_children; ++i) {
            if (read_from_disk(inode->directory.children_offsets[i], (void *)child_inode, inode_aligned_size) < 0) {
                free(child_inode);
                free(path_cpy);
                free(inode);

                return NULL;
            }

            if (strcmp(child_inode->name, component) == 0) {
                cpnt_found = true;
                break;
            }
        }

        if (!cpnt_found) {
            free(child_inode);
            free(path_cpy);
            free(inode);

            return NULL;
        }

        memcpy((void *)inode, (void *)child_inode, inode_aligned_size);
        component = strtok(NULL, "/");
    }

    free(path_cpy);
    free(child_inode);

    return inode;
}

int read_from_disk(disk_off_t disk_offset, void *buffer, size_t size)
{
    /* Check args */
    if (!buffer) return -1;

    if (disk_offset + size > DISK_CAPACITY_BYTES) return -1;

    if (size == 0) return 0;

    size_t disk_no = disk_offset / DISK_BLK_SIZE;
    size_t disk_check_no = (disk_offset + size) / DISK_BLK_SIZE;
    disk_check_no = (disk_offset + size) % DISK_BLK_SIZE == 0 ? disk_check_no - 1 : disk_check_no;

    if (disk_check_no != disk_no) return -1;
    
    char *blk_buf = (char *)malloc(DISK_BLK_SIZE);

    if (get_block(disk_no*DISK_BLK_SIZE, (void *)blk_buf) < 0) {
        free(blk_buf);

        return -1;
    }

    /* Read from buffer */
    size_t in_blk_offset = disk_offset - disk_no*DISK_BLK_SIZE;

    memcpy(buffer, (void *)(blk_buf + in_blk_offset), size);
    free(blk_buf);

    return 0;
}

/* Implementation of going to the disk, getting block 0, and returning a pointer to
 * a formatted superblock.
 */
struct lab3_superblock *get_disk_superblock(void)
{
    struct lab3_superblock *sblk = (struct lab3_superblock *)malloc(sizeof(struct lab3_superblock));

    /* Read block 0 from the disk */
    int rcode = read_from_disk(0, sblk, sizeof(struct lab3_superblock));
    if (rcode < 0) {
        free(sblk);
        return NULL;
    }

    return sblk;
}

int sanitize_fd_and_size(int fd, size_t size)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        return -1;
    }

    /* Check that there is a file with this descriptor */
    if (open_file_table[fd].inode == NULL) {
        return -1;
    }

    /* size is never negative because it's unsigned */
    if (size >= MAX_DATA_BLOCKS_PER_INODE * DISK_BLK_SIZE) {
        return -1;
    }

    return 0;
}

