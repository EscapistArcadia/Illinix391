#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"

#define KEYBOARD_IRQ        1       /* the irq # of keyboard in i8259*/
#define KEYBOARD_INTR_INDEX 0x21    /* interrupt # for keyboard interrupt */

/**
 * @brief enables keyboard interrupt
 */
void keyboard_init();

/**
 * @brief handles keyboard interrupts, based on scan code
 */
void keyboard_handler();

#endif
