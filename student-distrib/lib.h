/* lib.h - Defines for useful library functions
 * vim:ts=4 noexpandtab
 */

#ifndef _LIB_H
#define _LIB_H

#include "types.h"
#include "x86_desc.h"

/**
 * @brief \c file_operations_t stores function pointers to some sepcific type of files.
 */
typedef struct file_operations_t {
    int32_t (*open)(const uint8_t *file_name);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void *buf, uint32_t count);
    int32_t (*write)(int32_t fd, const void *buf, uint32_t count);
} file_operations_t;

/**
 * @brief \c file_t stores information of a file
 */
typedef struct file_t {
    file_operations_t *ops;
    uint32_t inode;
    uint32_t file_pos;
    uint32_t present;
} file_t;

typedef struct pcb_t {
    uint8_t present;
    uint8_t vidmap;
    uint8_t rtc;
    uint8_t rtc_fired;
    uint32_t rtc_curr;
    uint32_t rtc_rate;
    int32_t pid;
    struct pcb_t *parent;       /* parent's pcb */
    uint32_t ebp;               /* ebp for scheduling */
    uint32_t parent_ebp;        /* parent's ebp as the program quit */
    uint32_t esp0;              /* the tss.esp0 for the process */
    uint8_t argv[128];          /* argument passed by the user */
    file_t files[8];            /* files opened by the process */
} pcb_t;

/**
 * @brief pcb of the executing process
 */
pcb_t *get_current_pcb();

typedef struct terminal_t {
    int32_t pid;
    struct {
        uint8_t content[MAX_TERMINAL];
        uint32_t length;
        uint32_t in_progress;           /* if user is inputing in this terminal */
        uint32_t to_be_halt;            /* if user pressed Ctrl + C */
    } input;
    struct {
        int x;
        int y;
    } cursor;
} terminal_t;

extern terminal_t terms[TERMINAL_COUNT];
extern uint32_t shown_term_id;                 /* index of terminal showing on the screen */
extern uint32_t active_term_id;                /* index of terminal executing on the core */

int32_t printf(int8_t *format, ...);
void putc(uint8_t c);
void echo(uint8_t c);
int32_t puts(int8_t *s);
int8_t *itoa(uint32_t value, int8_t* buf, int32_t radix);
int8_t *strrev(int8_t* s);
uint32_t strlen(const int8_t* s);
void clear(void);
void scroll(void);

void get_screen_coordinate(int *x, int *y);
void set_screen_coordinate(int x, int y);
void get_cursor_pos(int *x, int *y);
void set_cursor_pos(int x, int y);
void switch_terminal(uint32_t next_id);

void* memset(void* s, int32_t c, uint32_t n);
void* memset_word(void* s, int32_t c, uint32_t n);
void* memset_dword(void* s, int32_t c, uint32_t n);
void* memcpy(void* dest, const void* src, uint32_t n);
void* memmove(void* dest, const void* src, uint32_t n);
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n);
int8_t* strcpy(int8_t* dest, const int8_t*src);
int8_t* strncpy(int8_t* dest, const int8_t*src, uint32_t n);

/* Userspace address-check functions */
int32_t bad_userspace_addr(const void* addr, int32_t len);
int32_t safe_strncpy(int8_t* dest, const int8_t* src, int32_t n);

/* Port read functions */
/* Inb reads a byte and returns its value as a zero-extended 32-bit
 * unsigned int */
static inline uint32_t inb(port) {
    uint32_t val;
    asm volatile ("             \n\
            xorl %0, %0         \n\
            inb  (%w1), %b0     \n\
            "
            : "=a"(val)
            : "d"(port)
            : "memory"
    );
    return val;
}

/* Reads two bytes from two consecutive ports, starting at "port",
 * concatenates them little-endian style, and returns them zero-extended
 * */
static inline uint32_t inw(port) {
    uint32_t val;
    asm volatile ("             \n\
            xorl %0, %0         \n\
            inw  (%w1), %w0     \n\
            "
            : "=a"(val)
            : "d"(port)
            : "memory"
    );
    return val;
}

/* Reads four bytes from four consecutive ports, starting at "port",
 * concatenates them little-endian style, and returns them */
static inline uint32_t inl(port) {
    uint32_t val;
    asm volatile ("inl (%w1), %0"
            : "=a"(val)
            : "d"(port)
            : "memory"
    );
    return val;
}

/* Writes a byte to a port */
#define outb(data, port)                \
do {                                    \
    asm volatile ("outb %b1, (%w0)"     \
            :                           \
            : "d"(port), "a"(data)      \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Writes two bytes to two consecutive ports */
#define outw(data, port)                \
do {                                    \
    asm volatile ("outw %w1, (%w0)"     \
            :                           \
            : "d"(port), "a"(data)      \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Writes four bytes to four consecutive ports */
#define outl(data, port)                \
do {                                    \
    asm volatile ("outl %l1, (%w0)"     \
            :                           \
            : "d"(port), "a"(data)      \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Clear interrupt flag - disables interrupts on this processor */
#define cli()                           \
do {                                    \
    asm volatile ("cli"                 \
            :                           \
            :                           \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Save flags and then clear interrupt flag
 * Saves the EFLAGS register into the variable "flags", and then
 * disables interrupts on this processor */
#define cli_and_save(flags)             \
do {                                    \
    asm volatile ("                   \n\
            pushfl                    \n\
            popl %0                   \n\
            cli                       \n\
            "                           \
            : "=r"(flags)               \
            :                           \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Set interrupt flag - enable interrupts on this processor */
#define sti()                           \
do {                                    \
    asm volatile ("sti"                 \
            :                           \
            :                           \
            : "memory", "cc"            \
    );                                  \
} while (0)

/* Restore flags
 * Puts the value in "flags" into the EFLAGS register.  Most often used
 * after a cli_and_save_flags(flags) */
#define restore_flags(flags)            \
do {                                    \
    asm volatile ("                   \n\
            pushl %0                  \n\
            popfl                     \n\
            "                           \
            :                           \
            : "r"(flags)                \
            : "memory", "cc"            \
    );                                  \
} while (0)

#endif /* _LIB_H */
