#ifndef _TERM_H
#define _TERM_H

#include "lib.h"

/**
 * @brief opens the terminal (does nothing)
 * 
 * @param path [ignored]
 * @return int32_t always 0
 */
int32_t terminal_open(const uint8_t* file);

/**
 * @brief closes the terminal (does nothing)
 * 
 * @param file [ignored]
 * @return int32_t always 0
 */
int32_t terminal_close(int32_t file);

/**
 * @brief reads the user input of the terminal to \p buf with capacity \p count
 * 
 * @param file [ignored] file descriptor
 * @param buf the input buffer
 * @param nbytes buffer size
 * @return int32_t number of bytes read
 */
int32_t terminal_read(int32_t file, void* buf, uint32_t count);

/**
 * @brief write first \p count of data from \p buf to the terminal
 * 
 * @param file [ignored]
 * @param buf data that will be written to the terminal
 * @param count # of bytes write to the terminal
 * @return int32_t # of bytes write
 */
int32_t terminal_write(int32_t file, const void* buf, uint32_t count);

#endif
