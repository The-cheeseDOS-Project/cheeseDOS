/*
 * cheeseDOS - My x86 DOS
 * Copyright (C) 2025  Connor Thomson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * cheeseDOS - My x86 DOS
 * Copyright (C) 2025  Connor Thomson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "rtc.h"
#include "io.h"
#include "stdbool.h"

#define CMOS_ADDRESS      0x70
#define CMOS_DATA         0x71

#define RTC_SECONDS       0x00
#define RTC_MINUTES       0x02
#define RTC_HOURS         0x04
#define RTC_DAY_OF_MONTH  0x07
#define RTC_MONTH         0x08
#define RTC_YEAR          0x09
#define RTC_STATUS_A      0x0A
#define RTC_STATUS_B      0x0B
#define RTC_CENTURY       0x32

#define TIMEZONE_OFFSET   0

static uint8_t get_rtc_register(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

static uint8_t bcd_to_bin(uint8_t bcd) {
    return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

void read_rtc_time(rtc_time_t* time) {
    uint8_t second, minute, hour, day, month, year, century;
    uint8_t status_b;
    bool is_bcd, is_12h;

    while (get_rtc_register(RTC_STATUS_A) & 0x80);

    uint8_t s1, s2, m1, m2, h1, h2, d1, d2, mo1, mo2, y1, y2, c1, c2;
    do {
        s1  = get_rtc_register(RTC_SECONDS);
        m1  = get_rtc_register(RTC_MINUTES);
        h1  = get_rtc_register(RTC_HOURS);
        d1  = get_rtc_register(RTC_DAY_OF_MONTH);
        mo1 = get_rtc_register(RTC_MONTH);
        y1  = get_rtc_register(RTC_YEAR);
        c1  = get_rtc_register(RTC_CENTURY);

        s2  = get_rtc_register(RTC_SECONDS);
        m2  = get_rtc_register(RTC_MINUTES);
        h2  = get_rtc_register(RTC_HOURS);
        d2  = get_rtc_register(RTC_DAY_OF_MONTH);
        mo2 = get_rtc_register(RTC_MONTH);
        y2  = get_rtc_register(RTC_YEAR);
        c2  = get_rtc_register(RTC_CENTURY);
    } while (s1 != s2 || m1 != m2 || h1 != h2 || d1 != d2 || mo1 != mo2 || y1 != y2 || c1 != c2);

    second  = s2;
    minute  = m2;
    hour    = h2;
    day     = d2;
    month   = mo2;
    year    = y2;
    century = c2;

    status_b = get_rtc_register(RTC_STATUS_B);
    is_bcd = !(status_b & 0x04);
    is_12h = !(status_b & 0x02);

    if (is_bcd) {
        second  = bcd_to_bin(second);
        minute  = bcd_to_bin(minute);
        hour    = bcd_to_bin(hour & 0x7F);
        day     = bcd_to_bin(day);
        month   = bcd_to_bin(month);
        year    = bcd_to_bin(year);
        century = bcd_to_bin(century);
    } else {
        hour = hour & 0x7F;
    }

    if (is_12h) {
        if (hour == 12) hour = 0;
        if (get_rtc_register(RTC_HOURS) & 0x80) hour += 12;
    }

    if (century == 0x00 || century == 0xFF) {
        century = (year < 70) ? 20 : 19;
    }

    time->year   = century * 100 + year;
    time->month  = month;
    time->day    = day;
    time->hour   = (hour + TIMEZONE_OFFSET) % 24;
    time->minute = minute;
    time->second = second;
}
