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

    cli();
    outb(RTC_DISABLE_NMI | RTC_REG_A, RTC_COMMAND);
    char a = inb(RTC_DATA);
    outb(RTC_DISABLE_NMI | RTC_REG_A, RTC_COMMAND);
    outb((a & 0xF0) | rate, RTC_DATA);
    sti();
}

void rtc_handler() {
    outb(RTC_REG_C, RTC_COMMAND);
    inb(RTC_DATA);

    send_eoi(RTC_IRQ);
}
