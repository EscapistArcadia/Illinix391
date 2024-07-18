#include "rtc.h"
#include "i8259.h"

#define RTC_COMMAND     0x70
#define RTC_DATA        0x71

#define RTC_REG_A       0x0A
#define RTC_REG_B       0x0B
#define RTC_REG_C       0x0C
#define RTC_DISABLE_NMI 0x80

void rtc_init() {
    cli();
    outb(RTC_DISABLE_NMI | RTC_REG_B, RTC_COMMAND);
    char b = inb(RTC_DATA);
    outb(RTC_DISABLE_NMI | RTC_REG_B, RTC_COMMAND);
    outb(b | 0x40, RTC_DATA);       /* enables PIE bit in B*/
    enable_irq(RTC_IRQ);
    sti();
}

void rtc_set_rate(uint32_t rate) {
    if (rate < 2 || rate > 15) {
        return;
    }

    /* reference: osdev */
    cli();
    outb(RTC_DISABLE_NMI | RTC_REG_A, RTC_COMMAND);
    char a = inb(RTC_DATA);
    outb(RTC_DISABLE_NMI | RTC_REG_A, RTC_COMMAND);
    outb((a & 0xF0) | rate, RTC_DATA);
    sti();
}

uint32_t rtc_fired = 0;             /* records the status of the rtc */

void rtc_handler() {
    outb(RTC_REG_C, RTC_COMMAND);
    inb(RTC_DATA);                  /* refreshes the RTC, or squeezed*/

    rtc_fired = 1;

    send_eoi(RTC_IRQ);
}

/**
 * @brief initializes RTC
 * 
 * @param path [ignored]
 * @return 0
 */
int32_t rtc_open(const uint8_t *path) {
    rtc_set_rate(RTC_MAX_RATE);
    return 0;
}

/**
 * @brief closes RTC
 * 
 * @param fd [ignored]
 * @return 0
 */
int32_t rtc_close(int32_t fd) {
    return 0;
}

/**
 * @brief returned when rtc is fired
 * 
 * @param fd [ignored]
 * @param buf [ignored]
 * @param count [ignored]
 * @return [ignored] when rtc is fired
 */
int32_t rtc_read(int32_t fd, void *buf, uint32_t count) {
    while (!rtc_fired);
    rtc_fired = 0;
    return 0;
}

/**
 * @brief updates rtc's frequency/rate
 * 
 * @param fd [ignored]
 * @param buf the address to new frequency
 * @param count MUST BE 4
 * @return 0 if success, nonzero otherwise
 */
int32_t rtc_write(int32_t fd, const void *buf, uint32_t count) {
    if (count != sizeof(int32_t) || buf == NULL) { /* the size or the buffer is invalid */
        return -1;
    }
    int32_t freq = *((int32_t*)buf);
    if (freq < RTC_MIN_FREQ             /* frequency < 2Hz */
        || freq > RTC_MAX_FREQ          /* frequency > 512 Hz */
        || (freq & (freq - 1))) {       /* not power of 2 */
        return -1;
    }
    
    uint32_t log2;                      /* checks the log2 of power-of-2 */
    for (log2 = -1; freq; ++log2, freq = freq >> 1);
    rtc_set_rate(16 - log2);

    return 0;
}
