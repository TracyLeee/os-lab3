/*
 * @file seek.c
 * @brief Implementation of seek
 *
 * @author Matteo Rizzo
 */
#include "fs_api.h"
#include "open_file_table.h"

int lab3_seek(int fd, uint32_t offset)
{
    /* Invalid fd */
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;

    /* Fail if the file is not open */
    if (!open_file_table[fd].inode) return -1;

    if (open_file_table[fd].inode->is_directory > 0) return -1;

    /* Invalid offset */
    if (offset > open_file_table[fd].inode->file.size) return -1;

    /* Modify offset */
    open_file_table[fd].seek_offset = offset;

    return 0;
}
