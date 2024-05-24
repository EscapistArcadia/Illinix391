/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/**
 * i8259 PIC is one of the most important part of x86 architecture. i8259 is
 * to manage hardware-generated interrupts, like keyboard, RTC, etc., and
 * send appropriate interrupt to the OS. There are two PICs, master and slave.
 * Each PIC has two ports, command and data.
 */

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFB; /* IRQs 0-7, slave is always enabled */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/**
 * @brief Initializes i8259.
 *      - sends the initialization command to command ports (\c 0x20 & \c 0xA0)
 *      - sends three control words to data ports
 *      - sends initial masking
 */
void i8259_init(void) {
    outb(0xFF, MASTER_8259_DATA);           /* masks all interrupts at first*/
    outb(0xFF, SLAVE_8259_DATA);

    outb(ICW1, MASTER_8259_COMMAND);        /* sends initialize command to both PICs */
    outb(ICW1, SLAVE_8259_COMMAND);

    outb(ICW2_MASTER, MASTER_8259_DATA);    /* sends three words to both PICs*/
    outb(ICW2_SLAVE, SLAVE_8259_DATA);

    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);

    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

    outb(master_mask, MASTER_8259_DATA); /* mask all, except slave irq of master */
    outb(slave_mask, SLAVE_8259_DATA);
}

/**
 * @brief enables a specified interrupts handled by PIC.
 *
 * @param irq_num the irq number of interrupt
 */
void enable_irq(uint32_t irq_num) {
    if (irq_num > IRQ_MAX) {                /* invalid irq number */
        return;
    }

    if (irq_num <= IRQ_MASTER_MAX) {        /* irq 0 - 7: master*/
        master_mask &= ~(1 << irq_num);     /* update mask and send */
        outb(master_mask, MASTER_8259_DATA);
    } else {                                /* irq 8 - 15: slave*/
        slave_mask &= ~(1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/**
 * @brief disables a specific interrupts handled by PIC.
 *
 * @param irq_num the irq number of interrupt
 */
void disable_irq(uint32_t irq_num) {
    if (irq_num > IRQ_MAX) {                /* invalid irq number */
        return;
    }

    if (irq_num <= IRQ_MASTER_MAX) {        /* irq 0 - 7: master */
        master_mask |= (1 << irq_num);      /* update mask and send */
        outb(master_mask, MASTER_8259_DATA);
    } else {                                /* irq 8 - 15: slave */
        slave_mask |= (1 << (irq_num - 8));
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/**
 * @brief sends end-of-interrupt to PIC for further interrupts.
 * 
 * @param irq_num the irq number of ended interrupt
 */
void send_eoi(uint32_t irq_num) {
    if (irq_num > IRQ_MAX) {                /* invalid irq number */
        return;
    }

    if (irq_num > IRQ_MASTER_MAX) {        /* slave eoi: send both PICs */
        outb(EOI | (irq_num - 8), SLAVE_8259_COMMAND);
        outb(EOI | SLAVE_IRQ, MASTER_8259_COMMAND);
    } else {
        outb(EOI | irq_num, MASTER_8259_COMMAND);
    }
}
