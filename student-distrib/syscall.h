#ifndef _SYSCALL_H
#define _SYSCALL_H

#ifndef ASM

#include "lib.h"

#define EXECUTABLE_MAGIC        0x464C457F          /* the first four bytes of executable files */
#define MAX_PROCESS             6                   /* 1 terminal and 2 user applications */

#define KERNEL_STACK            0x800000
#define KERNEL_STACK_SIZE       0x2000

#define PROGRAM_IMAGE           0x08048000          /* user-level destination of user program */
#define PROGRAM_IMAGE_LIMIT     0x3B8000            /* page end of user's page entry */

#define USER_ENTRY              (0x8000000 >> 22)   /* user's page directory entry */
#define USER_STACK              0x8400000           /* starting address of user entry */

extern pcb_t *pcbs[MAX_PROCESS];

/**
 * @brief terminates the currently executing user program, with exit code \p status
 * 
 * @param status the status code for the kernel
 * @return 0 if success, -1 if fail
 */
extern int32_t halt(uint8_t status);

/**
 * @brief runs a user program with parameter(s) specified in \p command
 * 
 * @param command user-input command
 * @return 0 if success, -1 if fail
 */
extern int32_t execute(const uint8_t *command);

/**
 * @brief continues to read a file from the position last time, or
 * 0 for the first time
 * 
 * @param fd the file descriptor of the file to read
 * @param buf the destination buffer of file content
 * @param count the buffer's capacity
 * @return int32_t count of bytes read
 */
extern int32_t read(int32_t fd, void* buf, int32_t count);

/**
 * @brief continues to write a file from the position last time, or
 * 0 for the first time
 * 
 * @param fd the file descriptor of the file to read
 * @param buf the source buffer of content to write
 * @param count count of bytes intended to write
 * @return count of bytes wrote
 */
extern int32_t write(int32_t fd, const void* buf, int32_t count);

/**
 * @brief opens a file from the file system
 * 
 * @param file_name name of the file
 * @return int32_t the file descriptor to the process
 */
extern int32_t open(const uint8_t* file_name);

/**
 * @brief closes the opened file and releases the descriptor
 * 
 * @param fd the file descriptor
 * @return 0 if success, -1 if fail
 */
extern int32_t close(int32_t fd);

/**
 * @brief gets the arguments user passed to the program as the
 * program started
 * 
 * @param buf the destination buffer address
 * @param nbytes the capacity of the buffer
 * @return number of bytes read
 */
extern int32_t getargs(uint8_t* buf, int32_t count);

/**
 * @brief requested the kernel for writing content on the video
 * memory
 * 
 * @param start the address to write the address of video memory
 * entry
 * @return 0 if success, -1 if fails 
 */
extern int32_t vidmap(uint8_t** start);

extern int32_t set_handler(int32_t signum, void* handler_address);

extern int32_t sigreturn(void);

/**
 * @brief requested to create a new file with specified \p file_name
 * 
 * @param file_name the name of the file
 * @return descriptor of the file for the process
 */
extern int32_t create(const uint8_t *file_name);

/**
 * @brief delete an existing file from the file system
 * 
 * @param file_name the name of the file
 * @return 0 if success, -1 if fail
 */
extern int32_t delete(const uint8_t *file_name);

#endif

#endif
