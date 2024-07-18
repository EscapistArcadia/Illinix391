#include "term.h"

#include "x86_desc.h"

extern uint8_t input[MAX_TERMINAL];
extern uint32_t length;
extern volatile uint32_t input_in_progress;

/**
 * @brief opens the terminal (does nothing)
 * 
 * @param path [ignored]
 * @return int32_t always 0
 */
int32_t terminal_open(const uint8_t* file) {
    return 0;
}

/**
 * @brief closes the terminal (does nothing)
 * 
 * @param file [ignored]
 * @return int32_t always 0
 */
int32_t terminal_close(int32_t file) {
    return 0;
}

/**
 * @brief reads the user input of the terminal to \p buf with capacity \p count
 * 
 * @param file [ignored] file descriptor
 * @param buf the input buffer
 * @param nbytes buffer size
 * @return int32_t number of bytes read
 */
int32_t terminal_read(int32_t file, void* buf, uint32_t count) {
    if (!buf || !count) {
        return -1;
    }

    input_in_progress = 1;
    while (input_in_progress);

    if (count > length) {
        *((uint8_t *)buf + length) = 0;
        count = length;
    }

    memcpy(buf, input, count);
    memset(input, 0, MAX_TERMINAL);
    length = 0;
    input_in_progress = 1;
    return count;
}

/**
 * @brief write first \p count of data from \p buf to the terminal
 * 
 * @param file [ignored] file descriptor
 * @param buf data that will be written to the terminal
 * @param count # of bytes write to the terminal
 * @return int32_t # of bytes write
 */
int32_t terminal_write(int32_t file, const void* buf, uint32_t count) {
    if (!buf || !count) {
        return 0;
    }

    cli();
    int i;
    for (i = 0; i < count; ++i) {       /* writes all buf, even though '\0' */
        putc(((char *)buf)[i]);
    }
    memset(input, 0, MAX_TERMINAL);
    length = 0;                 /* clear the buffer! */
    sti();
    return i;
}
