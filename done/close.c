/*
 * @file close.c
 * @brief Implementation of close
 *
 * @author Matteo Rizzo
 */
#include <stdlib.h>

#include "fs_api.h"
#include "open_file_table.h"

int lab3_close(int fd)
{
    /* Invalid fd */
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;

    /* Fail if the file is not open */
    if (!open_file_table[fd].inode) return -1;

    free(open_file_table[fd].inode);
    open_file_table[fd].inode = NULL;

    return 0;
}
