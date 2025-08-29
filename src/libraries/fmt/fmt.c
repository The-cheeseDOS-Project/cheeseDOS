#include "fmt.h"


void print_uint(uint32_t num) {
    char buf[12];
    int i = 0;
    if (num == 0) {
        vga_putchar('0');
        return;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        vga_putchar(buf[j]);
    }
}
