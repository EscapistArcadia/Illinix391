#ifndef _RTC_H
#define _RTC_H

#include "lib.h"

#define RTC_IRQ             8       /* the irq # of keyboard in i8259*/
#define RTC_INTR_INDEX      0x28    /* interrupt # for keyboard interrupt */

/* initializes rtc and set to default frequency */
void rtc_init();

/* handles rtc interrupts */
void rtc_handler();

#endif
