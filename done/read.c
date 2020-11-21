/*
 * @file read.c
 * @brief Implementation of read
 *
 * @author Matteo Rizzo, Mark Sutherland
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disk.h"
#include "fs_api.h"
#include "fs_util.h"
#include "open_file_table.h"

int32_t lab3_read(int fd, void *buffer, uint32_t size)
{
    /* Invalid fd */
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;

    /* Fail if the file is not open */
    if (!open_file_table[fd].inode) return -1;

    if (!buffer) return -1;

    if (size > DISK_BLK_SIZE*MAX_DATA_BLOCKS_PER_INODE) return -1;

    struct lab3_inode *inode = open_file_table[fd].inode;
    uint32_t offset = open_file_table[fd].seek_offset;

    if (offset + size > inode->file.size) size = inode->file.size - offset;

    uint32_t cur = offset;
    uint32_t next = (offset / DISK_BLK_SIZE + 1)*DISK_BLK_SIZE;

    while (next <= offset + size) {
        size_t blk_idx = cur / DISK_BLK_SIZE;
        size_t buf_offset = cur - offset;

        if (read_from_disk(inode->file.data_block_offsets[blk_idx], 
                           (void *)((char *)buffer + buf_offset), next - cur) < 0) {
            return -1;
        }
        
        cur = next;
        next += DISK_BLK_SIZE;
    }

    if (offset + size > cur) {
        if (read_from_disk(inode->file.data_block_offsets[cur / DISK_BLK_SIZE], 
                           (void *)((char *)buffer + cur - offset), 
                           offset + size - cur) < 0) {
            return -1;
        }
    }

    open_file_table[fd].seek_offset = offset + size;

    return size;
}
