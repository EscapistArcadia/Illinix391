#ifndef _RTC_H
#define _RTC_H

#include "lib.h"

#define RTC_IRQ             8       /* the irq # of keyboard in i8259*/
#define RTC_INTR_INDEX      0x28    /* interrupt # for keyboard interrupt */

#define RTC_MIN_RATE    6
#define RTC_MAX_RATE    15

#define RTC_MAX_FREQ    (32768 >> (RTC_MIN_RATE - 1))
#define RTC_MIN_FREQ    (32768 >> (RTC_MAX_RATE - 1))

/* initializes rtc and set to default frequency */
void rtc_init();

/* handles rtc interrupts */
void rtc_handler();

/**
 * @brief initializes RTC
 * 
 * @param path [ignored]
 * @return 0
 */
int32_t rtc_open(const uint8_t *path);

/**
 * @brief closes RTC
 * 
 * @param fd [ignored]
 * @return 0
 */
int32_t rtc_close(int32_t fd);

/**
 * @brief returned when rtc is fired
 * 
 * @param fd [ignored]
 * @param buf [ignored]
 * @param count [ignored]
 * @return [ignored] when rtc is fired
 */
int32_t rtc_read(int32_t fd, void *buf, uint32_t count);

/**
 * @brief updates rtc's frequency/rate
 * 
 * @param fd [ignored]
 * @param buf the address to new frequency
 * @param count MUST BE 4
 * @return 0 if success, nonzero otherwise
 */
int32_t rtc_write(int32_t fd, const void *buf, uint32_t count);

#endif
