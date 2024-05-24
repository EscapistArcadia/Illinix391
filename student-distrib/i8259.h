/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_COMMAND 0x20
#define MASTER_8259_DATA    0x21
#define SLAVE_8259_COMMAND  0xA0
#define SLAVE_8259_DATA     0xA1

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60

#define IRQ_MIN             0x00
#define SLAVE_IRQ           0x02
#define IRQ_MASTER_MAX      0x07
#define IRQ_MAX             0x0F

/* Externally-visible functions */

/**
 * @brief Initializes i8259.
 *      - sends the initialization command to command ports (\c 0x20 & \c 0xA0)
 *      - sends three control words to data ports
 *      - sends initial masking
 */
void i8259_init(void);

/**
 * @brief enables a specified interrupts handled by PIC.
 *
 * @param irq_num the irq number of interrupt
 */
void enable_irq(uint32_t irq_num);

/**
 * @brief disables a specific interrupts handled by PIC.
 *
 * @param irq_num the irq number of interrupt
 */
void disable_irq(uint32_t irq_num);

/**
 * @brief sends end-of-interrupt to PIC for further interrupts.
 * 
 * @param irq_num the irq number of ended interrupt
 */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
