/*
 * cmos.c
 * A small driver to read the time and date from the RTC (Real Time Clock) for x86 systems.
 */

#include <kernel/config.h>

#ifdef CONFIG_ARCH_X86

#include <stdint.h>

#include <asm/port_io.h>

#include <drivers/timer.h>

#include <kernel/debug.h>

/* Probably will never have to change this. */
#define CURRENT_MILLENIUM	2000

/* I/O Ports */
#define CMOS_REGISTER	0x70
#define CMOS_DATA	0x71

#define SECONDS_REG		0
#define MINUTES_REG		2
#define HOURS_REG		4
#define WEEKDAY_REG		6
#define DAY_OF_MONTH_REG	7
#define MONTH_REG		8
#define YEAR_REG		9

#define STATUS_REG_A	0xA
#define STATUS_REG_B	0xB

#define TWENTY_FOUR_HOUR_FMT	(1 << 1)
#define BINARY_MODE		(1 << 2)

#define NMI_DISABLE	(1 << 7)

/* Bit 7 of the 'hour' value indicates PM if not in 24-hour time */
#define PM_BIT		(1 << 7)

uint32_t make_posix_time(uint32_t seconds, uint32_t hours, uint32_t mins, uint32_t day_of_month, uint32_t month, uint32_t year);

static inline uint8_t
cmos_read_register(uint8_t reg)
{
        outb(NMI_DISABLE | reg, CMOS_REGISTER);
        return inb(CMOS_DATA);
}

static inline uint8_t
bcd_to_binary(uint8_t bcd)
{
        return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0xF);
}

uint32_t
get_current_time(void)
{
        struct {
                uint32_t sec;
                uint32_t min;
                uint32_t hour;
                uint32_t day_of_month;
                uint32_t month;
                uint32_t year;
        } time, time_last;
        uint8_t status;

        time.sec = cmos_read_register(SECONDS_REG);
        time.min = cmos_read_register(MINUTES_REG);
        time.hour = cmos_read_register(HOURS_REG);
        time.day_of_month = cmos_read_register(DAY_OF_MONTH_REG);
        time.month = cmos_read_register(MONTH_REG);
        time.year = cmos_read_register(YEAR_REG);

        /*
         * The CMOS can give bizarre readings since it might be in the middle
         * of updating while it is being read. Hopefully this works.
         */
        do {
                time_last.sec = cmos_read_register(SECONDS_REG);
                time_last.min = cmos_read_register(MINUTES_REG);
                time_last.hour = cmos_read_register(HOURS_REG);
                time_last.day_of_month = cmos_read_register(DAY_OF_MONTH_REG);
                time_last.month = cmos_read_register(MONTH_REG);
                time_last.year = cmos_read_register(YEAR_REG);
        } while (time_last.sec != time.sec ||
                 time_last.min != time.min ||
                 time_last.hour != time.hour ||
                 time_last.day_of_month != time.day_of_month ||
                 time_last.month != time.month ||
                 time_last.year != time.year);

        status = inb(STATUS_REG_B);
        if (!(status & BINARY_MODE)) {
                time_last.sec = bcd_to_binary(time_last.sec);
                time_last.min = bcd_to_binary(time_last.min);
                time_last.hour = bcd_to_binary(time_last.hour);
                time_last.day_of_month = bcd_to_binary(time_last.day_of_month);
                time_last.month = bcd_to_binary(time_last.month);
                time_last.year = bcd_to_binary(time_last.year);
        }
        if (!(status & TWENTY_FOUR_HOUR_FMT) && (time_last.hour & PM_BIT))
                time_last.hour = (((time_last.hour & 0x7F) + 12) % 24);
        return make_posix_time(time_last.sec, time_last.min, time_last.hour, time_last.day_of_month, time_last.month, time_last.year + CURRENT_MILLENIUM);
}

#endif /* CONFIG_ARCH_X86 */
