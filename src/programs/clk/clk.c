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
#include "vga.h"
#include "stdint.h"

void clk(const char* *u) {
    (void)u;
    struct rtc_time current_time;
    read_rtc_time(&current_time);
    if (current_time.month < 10) vga_putchar('0');
    print_uint(current_time.month);
    vga_putchar('/');
    if (current_time.day < 10) vga_putchar('0');
    print_uint(current_time.day);
    vga_putchar('/');
    print_uint(current_time.year);
    print(" ");
    uint8_t display_hour = current_time.hour;
    const char* ampm = "AM";
    if (display_hour >= 12) {
        ampm = "PM";
        if (display_hour > 12) {
            display_hour -= 12;
        }
    } else if (display_hour == 0) {
        display_hour = 12;
    }
    if (display_hour < 10) vga_putchar('0');
    print_uint(display_hour);
    vga_putchar(':');
    if (current_time.minute < 10) vga_putchar('0');
    print_uint(current_time.minute);
    vga_putchar(':');
    if (current_time.second < 10) vga_putchar('0');
    print_uint(current_time.second);
    vga_putchar(' ');
    print(ampm);
    print("\n");
}
