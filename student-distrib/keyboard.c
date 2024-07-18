#include "keyboard.h"
#include "syscall.h"
#include "i8259.h"

#define KEYBOARD_PORT       0x60    /* the port for keyboard */

#define SC_ESCAPE           0x01
#define SC_BACKSPACE        0x0E
#define SC_TAB              0x0F
#define SC_ENTER            0x1C
#define SC_LEFTCONTROL      0x1D
#define SC_LEFTSHIFT        0x2A
#define SC_RIGHTSHIFT       0x36
#define SC_LEFTALT          0x38
#define SC_CAPSLOCK         0x3A
#define SC_F1               0x3B
#define SC_F2               0x3C
#define SC_F3               0x3D
#define SC_F4               0x3E
#define SC_F5               0x3F
#define SC_F6               0x40
#define SC_F7               0x41
#define SC_F8               0x42
#define SC_F9               0x43
#define SC_F10              0x44
#define SC_NUMBERLOCK       0x45
#define SC_SCROLLLOCK       0x46
#define SC_F11              0x57
#define SC_F12              0x58

#define SC_LEFTCONTROLREL   0x9D
#define SC_LEFTSHIFTREL     0xAA
#define SC_RIGHTSHIFTREL    0xB6

#define KBF_LEFTSHIFT       0x0001
#define KBF_RIGHTSHIFT      0x0002
#define KBF_ALT             0x0004
#define KBF_CAPSLOCK        0x0008
#define KBF_CONTROL         0x0010

uint32_t keyboard_bitmap = 0x0;

char visible_scancode_map[] = {
    '\0','\0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=','\0','\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']','\0','\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`','\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/','\0', '\0',
    '\0', ' '
};

char visible_scancode_map_shifted[] = {
    '\0','\0', '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+','\0','\0',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '[', ']','\0','\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '\"', '~','\0', '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?','\0', '\0',
    '\0', ' '
};

uint8_t input[MAX_TERMINAL];            /* records user typed letters*/
uint32_t length;                        /* records length of input */
volatile uint32_t input_in_progress;    /* nonzero if user is typing */

/**
 * @brief enables keyboard interrupt
 */
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ);
}

/**
 * @brief handles keyboard interrupts, based on scan code
 */
void keyboard_handler() {
    unsigned char scancode = inb(KEYBOARD_PORT);
    
    /* shift */
    if (scancode == SC_LEFTSHIFT) {
        keyboard_bitmap |= KBF_LEFTSHIFT;
    } else if (scancode == SC_RIGHTSHIFT) {
        keyboard_bitmap |= KBF_RIGHTSHIFT;
    } else if (scancode == SC_LEFTSHIFTREL || scancode == SC_RIGHTSHIFTREL) {
        keyboard_bitmap &= ~(KBF_LEFTSHIFT | KBF_RIGHTSHIFT);
    }
    /* caps lock */
    else if (scancode == SC_CAPSLOCK) {
        keyboard_bitmap &= ~(KBF_CAPSLOCK);
    }
    /* control */
    else if (scancode == SC_LEFTCONTROL) {
        keyboard_bitmap |= KBF_CONTROL;
    } else if (scancode == SC_LEFTCONTROLREL) {
        keyboard_bitmap &= ~KBF_CONTROL;
    }
    /* tab */
    else if (scancode == SC_TAB) {
        putc('\t');
        input[length++] = '\t';
    }
    /* backspace */
    else if (scancode == SC_BACKSPACE) {
        if (length > 0) {
            putc('\b');
            input[--length] = 0;
        }
    }
    /* enter */
    else if (scancode == SC_ENTER) {
        putc('\n');
        input_in_progress = 0;
    }
    /* default visible chars */
    else {
        if (scancode < sizeof(visible_scancode_map) && visible_scancode_map[scancode] != 0) {
            if (keyboard_bitmap & KBF_CONTROL) {    /* control is pressed */
                if (scancode == 0x26) {             /* Ctrl+L: clean the screen */
                    clear();
                } else if (scancode == 0x2E) {
                    memset(input, 0, MAX_TERMINAL);
                    send_eoi(KEYBOARD_IRQ);
                    halt(220);
                }
            } else {
                char ch = (keyboard_bitmap & KBF_LEFTSHIFT || keyboard_bitmap & KBF_RIGHTSHIFT
                            ? visible_scancode_map_shifted : visible_scancode_map)[scancode];
                putc(ch);                /* checks if shift is down */
                if (length < MAX_TERMINAL) {
                    input[length++] = ch;
                }
            }
        }
    }

    send_eoi(KEYBOARD_IRQ);
}
