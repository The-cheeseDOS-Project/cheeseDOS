#include "rtc.h"
#include "io.h"
#include <stdbool.h>

#define CMOS_ADDRESS    0x70
#define CMOS_DATA       0x71

#define RTC_SECONDS     0x00
#define RTC_MINUTES     0x02
#define RTC_HOURS       0x04
#define RTC_DAY_OF_WEEK 0x06
#define RTC_DAY_OF_MONTH 0x07
#define RTC_MONTH       0x08
#define RTC_YEAR        0x09
#define RTC_STATUS_A    0x0A
#define RTC_STATUS_B    0x0B
#define RTC_CENTURY     0x32

static uint8_t bcd_to_bin(uint8_t bcd) {
    return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

static uint8_t get_rtc_register(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

void read_rtc_time(rtc_time_t* time) {
    uint8_t second, minute, hour, day, month, year, century;
    uint8_t register_b;
    bool is_bcd;
    bool is_12_hour_mode;

    while (get_rtc_register(RTC_STATUS_A) & 0x80);

    second  = get_rtc_register(RTC_SECONDS);
    minute  = get_rtc_register(RTC_MINUTES);
    hour    = get_rtc_register(RTC_HOURS);
    day     = get_rtc_register(RTC_DAY_OF_MONTH);
    month   = get_rtc_register(RTC_MONTH);
    year    = get_rtc_register(RTC_YEAR);
    century = get_rtc_register(RTC_CENTURY);

    register_b = get_rtc_register(RTC_STATUS_B);

    is_bcd = !(register_b & 0x04);
    is_12_hour_mode = !(register_b & 0x02);

    if (is_bcd) {
        second  = bcd_to_bin(second);
        minute  = bcd_to_bin(minute);
        hour    = bcd_to_bin(hour);
        day     = bcd_to_bin(day);
        month   = bcd_to_bin(month);
        year    = bcd_to_bin(year);
        century = bcd_to_bin(century);
    }

    if (is_12_hour_mode && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    } else if (is_12_hour_mode && !(hour & 0x80) && hour == 12) {
        hour = 0;
    }

    if (year < 70) {
        time->year = 2000 + year;
    } else {
        time->year = 1900 + year;
    }

    time->second = second;
    time->minute = minute;
    time->hour   = hour;
    time->day    = day;
    time->month  = month;
}
