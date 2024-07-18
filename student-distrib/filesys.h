#ifndef _FILESYS_H
#define _FILESYS_H

#include "lib.h"

#define FS_BLOCK_SIZE (4 << 10)     /* 4kb */
#define FS_MAX_LEN 32
#define DENTRY_COUNT 63

typedef struct {
    uint32_t file_size;             /* in bytes */
    uint32_t data_blocks[1023];     /* (4096 - sizeof(uint32_t)) / 4 */
} inode_t;

typedef struct {
    uint8_t file_name[32];
    uint32_t file_type;
    uint32_t inode_num;
    uint8_t reserved[24];
} dentry_t;

typedef struct {
    uint8_t data[FS_BLOCK_SIZE];
} data_block_t;

typedef struct {
    uint32_t dentry_count;
    uint32_t inode_count;
    uint32_t data_block_count;
    uint8_t reserved[52];
    dentry_t dentries[63];          /* (4096 - 64) / 64 */
} boot_block_t;

/**
 * @brief initializes the file system
 * 
 * @param start the boot block address
 */
void file_system_init(uint32_t start);

/**
 * @brief opens a file at \p path
 * 
 * @param file_names the directory of the file
 * @return 0 if success, 1 if fail
 */
int32_t file_open(const uint8_t *file_name);

/**
 * @brief closes the file at \p fd
 * 
 * @param fd the file descriptor
 * @return 0 if success, 1 if fail
 */
int32_t file_close(int32_t fd);

/**
 * @brief reads the data of the file at \p fd
 * 
 * @param fd the descriptor
 * @param buf the buffer to read file
 * @param count the capacity of \p buf
 * @return number of bytes read
 */
int32_t file_read(int32_t fd, void *buf, uint32_t count);

/**
 * @brief writes the first \p count bytes at \p buf to the file at \p fd
 * 
 * @param fd the descriptor
 * @param buf the data to write
 * @param count counts of buffer write
 * @return -1 TODO: make file system writable.
 */
int32_t file_write(int32_t fd, const void *buf, uint32_t count);

/**
 * @brief opens a directory at \p path
 * 
 * @param file_names the directory of the file
 * @return 0 if success, 1 if fail
 */
int32_t dir_open(const uint8_t *file_name);

/**
 * @brief closes the directory at \p fd
 * 
 * @param fd the file descriptor
 * @return 0 if success, 1 if fail
 */
int32_t dir_close(int32_t fd);

/**
 * @brief reads the data of the directory at \p fd
 * 
 * @param fd the descriptor
 * @param buf the buffer to read directory
 * @param count the capacity of \p buf
 * @return number of bytes read
 */
int32_t dir_read(int32_t fd, void *buf, uint32_t count);

/**
 * @brief writes the first \p count bytes at \p buf to the directory at \p fd
 * 
 * @param fd the descriptor
 * @param buf the data to write
 * @param count counts of buffer write
 * @return -1 TODO: make file system writable.
 */
int32_t dir_write(int32_t fd, const void *buf, uint32_t count);


/**
 * @brief gets the size of the file at \p fd
 * 
 * @param fd the descriptor
 * @return the size of file in bytes
 */
int32_t file_size(int32_t fd);

#endif
