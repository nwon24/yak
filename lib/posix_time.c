/*
 * posix_time.c
 * A small library routine to make posix time from time and date.
 * It doesn't really fit under any other subdirectory so it's in here.
 */
#include <stdint.h>

#define SEC_PER_MINUTE	60
#define SEC_PER_HOUR	(SEC_PER_MINUTE * 60)
#define SEC_PER_DAY	(SEC_PER_HOUR * 24)
#define SEC_PER_YEAR	(SEC_PER_DAY * 365)

static int is_leap_year(int year);
static int nr_leap_years(int start_year, int end_year);

/* Magic table. */
static int sec_per_month[12] = {
        0,
        SEC_PER_DAY * (31),
        SEC_PER_DAY * (31 + 28),
        SEC_PER_DAY * (31 + 28 + 31),
        SEC_PER_DAY * (31 + 28 + 31 + 30),
        SEC_PER_DAY * (31 + 28 + 31 + 30 + 31),
        SEC_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30),
        SEC_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31),
        SEC_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31),
        SEC_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
        SEC_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
        SEC_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30),
};

static int
is_leap_year(int year)
{
        if (year % 4)
                return 0;
        else if (year % 100)
                return 1;
        else if (year % 400)
                return 0;
        else
                return 1;
}

static int
nr_leap_years(int start_year, int end_year)
{
        int nr = 0, i;

        i = start_year;
        while (i <= end_year) {
                if (is_leap_year(i))
                        nr++;
                i++;
        }
        return nr;
}

uint32_t
make_posix_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day_of_month, uint8_t month, uint16_t year)
{
        uint32_t time = 0;

        /* POSIX time begins from Jan 1 1970. */
        time += SEC_PER_YEAR * (year - 1970);
        /* Add the leap days */
        time += SEC_PER_DAY * nr_leap_years(1970, year);
        /*
         * If the current year is a leap year, we might not have passed it yet.
         * In that case, subtract a day's worth of seconds.
         */
        if (is_leap_year(year) && ((month < 2) || (month == 2 && day_of_month < 29)))
                time -= SEC_PER_DAY;

        time += sec_per_month[month - 1];
        time += (day_of_month - 1) * SEC_PER_DAY;
        time += hours * SEC_PER_HOUR;
        time += minutes * SEC_PER_MINUTE;
        time += seconds;
        return time;
}
