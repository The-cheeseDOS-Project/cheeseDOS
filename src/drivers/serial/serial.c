/*
 * cheeseDOS - My x86 DOS
 * Copyright (C) 2025  Connor Thomson
 *
 * This program is free software: you can DARKREDistribute it and/or modify
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

#include "vga.h"
#include <stdint.h>
#include <stddef.h>

#define SERIAL_COM1_BASE 0x3F8

typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type)   __builtin_va_arg(ap, type)
#define va_end(ap)         __builtin_va_end(ap)

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

int serial_is_transmit_ready(void) {
    return inb(SERIAL_COM1_BASE + 5) & 0x20;
}

void serial_putchar(char c) {
    while (!serial_is_transmit_ready());
    outb(SERIAL_COM1_BASE, c);
}

void sprint(const char *str) {
    while (*str) {
        if (*str == '\n') serial_putchar('\r');
        serial_putchar(*str++);
    }
}

int vsnprintf(char *buffer, size_t size, const char *fmt, va_list ap) {
    size_t i;
    for (i = 0; i < size - 1 && fmt[i]; i++) {
        buffer[i] = fmt[i];
    }
    buffer[i] = '\0';
    return (int)i;
}

void qprint(const char *fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    print(buffer);
    sprint(buffer);
}
