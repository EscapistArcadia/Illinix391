#include "filesys.h"

boot_block_t *boot_block;
inode_t *inode_blocks;
data_block_t *data_blocks;

/**
 * @brief initializes the file system
 * 
 * @param start the boot block address
 */
void file_system_init(uint32_t start) {
    boot_block = (boot_block_t *)start;         /* records the starting address */
    inode_blocks = (inode_t *)(boot_block + 1); /* skips the boot block */
    data_blocks = (data_block_t *)(boot_block) + boot_block->inode_count + 1;
}

/**
 * @brief reads the dentry corresponding to \p file_name
 * 
 * @param file_name 
 * @param dentry the dentry returned
 * @return 0 if succeed, -1 if fail
 */
int32_t read_dentry_by_name(const uint8_t *file_name, dentry_t *dentry) {
    if (file_name == NULL) {
        return -1;
    }

    uint32_t i, j;
    dentry_t *pos;
    const uint8_t *left, *right;
    
    /* compares file_name to file name in each dentry */
    for (i = 0, pos = boot_block->dentries; i < boot_block->dentry_count; ++i, ++pos) {
        /* the file name should be short enough, and matches with the parameter */
        for (j = 0, left = file_name, right = pos->file_name;
             !(j & (~(FS_MAX_LEN - 1))) && *left && *left == *right;
             ++j, ++left, ++right);
        if (!*left && (!*right || j == FS_MAX_LEN)) {
            memcpy(dentry, pos, sizeof(dentry_t));
            return 0;
        }
    }
    return -1;
}

/**
 * @brief reads the dentry block at \p index
 * 
 * @param index 
 * @param dentry the dentry at \p index
 * @return 0 if succeed, -1 if fail
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry) {
    if (index >= boot_block->dentry_count) {
        return -1;
    }
    dentry = boot_block->dentries + index;
    return 0;
}

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
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t len) {
    if (!buf || !len || inode >= boot_block->inode_count) {
        return -1;
    }

    inode_t *in = inode_blocks + inode;
    if (offset >= in->file_size) {                  /* offset is too large */
        return 0;
    }
    
    uint32_t remain = in->file_size - offset;
    if (remain < len) {                             /* buffer is too large */
        len = remain;                               /* validize the len */
    }

    uint32_t index = offset >> 12;                  /* offset / 4096 */
    offset &= (FS_BLOCK_SIZE - 1);                  /* offset % 4096 */

    uint32_t *block = in->data_blocks + index;
    remain = FS_BLOCK_SIZE - offset;
    if (remain >= len) {                            /* only one block */
        memcpy(buf, data_blocks[*block].data + offset, len);     /* don't need to browse blocks */
        return len;
    } else {
        uint8_t *pos = buf + remain;                /* records the position */
        memcpy(buf, data_blocks[*(block++)].data + offset, remain);
        for (remain = len - remain;
             remain & (~(FS_BLOCK_SIZE - 1));
             ++block, remain -= FS_BLOCK_SIZE, pos += FS_BLOCK_SIZE) {
            memcpy(pos, data_blocks[*block].data, FS_BLOCK_SIZE);
        }
        memcpy(pos, data_blocks[*block].data, remain);
        return len;
    }
}

/**
 * @brief opens a file at \p path
 * 
 * @param path the directory of the file
 * @return inode number, ONLY FOR CP2
 */
int32_t file_open(const uint8_t *file_name) {
    dentry_t dentry;
    if (read_dentry_by_name(file_name, &dentry) == -1) {
        return -1;
    }
    return dentry.inode_num;
}

/**
 * @brief closes the file at \p fd
 * 
 * @param fd the file descriptor
 * @return 0 if success, 1 if fail
 */
int32_t file_close(int32_t fd) {
    return 0;
}

/**
 * @brief reads the data of the file at \p fd
 * 
 * @param fd the descriptor
 * @param buf the buffer to read file
 * @param count the capacity of \p buf
 * @return number of bytes read
 */
int32_t file_read(int32_t fd, void *buf, uint32_t count) {
    pcb_t *curr = get_current_pcb();
    return read_data(curr->files[fd].inode, curr->files[fd].file_pos, buf, count);
}

/**
 * @brief writes the first \p count bytes at \p buf to the file at \p fd
 * 
 * @param fd the descriptor
 * @param buf the data to write
 * @param count counts of buffer write
 * @return -1 TODO: make file system writable.
 */
int32_t file_write(int32_t fd, const void *buf, uint32_t count) {
    return -1;
}

/**
 * @brief opens a directory at \p path
 * 
 * @param file_names the directory of the file
 * @return 0 if success, 1 if fail
 */
int32_t dir_open(const uint8_t *file_name) {
    dentry_t dentry;
    if (read_dentry_by_name(file_name, &dentry) == -1) {
        return -1;
    }
    return 0;
}

/**
 * @brief closes the directory at \p fd
 * 
 * @param fd the file descriptor
 * @return 0 if success, 1 if fail
 */
int32_t dir_close(int32_t fd) {
    return 0;
}

/**
 * @brief reads the data of the directory at \p fd
 * 
 * @param fd the descriptor
 * @param buf the buffer to read directory
 * @param count the capacity of \p buf
 * @return number of bytes read
 */
int32_t dir_read(int32_t fd, void *buf, uint32_t count) {
    static uint32_t pos = 0;
    static const uint32_t max_count = FS_MAX_LEN + 1;       /* reserves a byte for \0 */
    if (buf == NULL || pos >= boot_block->dentry_count) {
        pos = 0;
        return 0;
    }

    dentry_t de = boot_block->dentries[pos];
    if (count > max_count) {
        count = max_count;
    }
    memcpy(buf, de.file_name, count);
    ++pos;
    return count;
}

/**
 * @brief writes the first \p count bytes at \p buf to the directory at \p fd
 * 
 * @param fd the descriptor
 * @param buf the data to write
 * @param count counts of buffer write
 * @return -1 TODO: make file system writable.
 */
int32_t dir_write(int32_t fd, const void *buf, uint32_t count) {
    return -1;
}

/**
 * @brief gets the size of the file at \p fd
 * 
 * @param fd the descriptor
 * @return the size of file in bytes
 */
int32_t file_size(int32_t fd) {
    return inode_blocks[fd].file_size;
}
