#include "programs.h"
#include "rtc.h"
#include "vga.h"

void rtc(const char*) {
    rtc_time_t current_time;
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