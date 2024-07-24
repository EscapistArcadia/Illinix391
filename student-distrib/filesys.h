#ifndef _FILESYS_H
#define _FILESYS_H

#include "lib.h"
#include "x86_desc.h"

#define FS_BLOCK_SIZE (4 << 10)     /* 4kb */
#define FS_MAX_LEN 32
#define DENTRY_COUNT 63

typedef struct {
    uint32_t file_size;             /* in bytes */
    uint32_t data_blocks[1023];     /* (4096 - sizeof(uint32_t)) / 4 */
} inode_t;

typedef struct dentry_t {
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

boot_block_t *boot_block;
inode_t *inode_blocks;
data_block_t *data_blocks;

uint8_t inode_bitmap[64];
uint8_t data_block_bitmap[64];

/**
 * @brief reads \p count of sectors starting at \p index to \p buf
 * the buffer size should be count * 512 (ATA_SECTOR_SIZE)
 * 
 * @param index the starting index of the sector to read
 * @param count the count of sectors to read
 * @param buf the destination buffer
 * @return count of bytes read
 */
uint32_t read_ata_sectors(uint32_t index, uint32_t count, uint8_t *buf);

/**
 * @brief writes \p count of sectors starting at \p index from \p buf
 * the buffer size should be count * 512 (ATA_SECTOR_SIZE)
 * 
 * @param index the starting index of the sector to write
 * @param count the count of sectors to write
 * @param buf the source buffer
 * @return count of bytes written
 */
uint32_t write_ata_sectors(uint32_t index, uint32_t count, const uint8_t *buf);

/**
 * @brief initializes the file system
 * 
 * @param start the boot block address
 */
void file_system_init(uint32_t start);

/**
 * @brief reads the dentry corresponding to \p file_name
 * 
 * @param file_name 
 * @param dentry the dentry returned
 * @return 0 if succeed, -1 if fail
 */
int32_t read_dentry_by_name(const uint8_t *file_name, dentry_t *dentry);

/**
 * @brief reads the dentry block at \p index
 * 
 * @param index 
 * @param dentry the dentry at \p index
 * @return 0 if succeed, -1 if fail
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);

/**
 * @brief reads the data starting from \p offset of \p inode to \p buf
 * with capacity \p len
 * 
 * @param inode the inode number to read
 * @param offset the starting position to read
 * @param buf the buffer to write
 * @param len the capacity of the buffer
 * @return number of bytes read, or -1 if fail
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t len);

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
