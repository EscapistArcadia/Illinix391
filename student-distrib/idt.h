#ifndef _IDT_H
#define _IDT_H

#include "lib.h"

/**
 * @brief initializes IDT for the OS and loads it to IDTR
 */
void idt_init();

#endif
