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

#include "mem.h"

static void ram_used() {
    uint32_t used = heap_ptr - (uint32_t)&_init_start;
    uint32_t used_kb = used / 1024;
    char buf[16];
    itoa(used_kb, buf, 10);
    print(buf);
    print("K");
}

static void ram_allocated() {
    unsigned char low, high;
    unsigned short ext_kb, total_kb, total_mb;

    outb(0x70, 0x30); low = inb(0x71);
    outb(0x70, 0x31); high = inb(0x71);

    ext_kb = ((unsigned short)high << 8) | low;
    total_kb = 640 + ext_kb;
    total_mb = total_kb / 1024;

    unsigned short val = total_kb;
    unsigned short div = 10000;
    int started = 0;
    while (div > 0) {
        char digit = (val / div) % 10;
        if (digit || started || div == 1) {
            char c[2] = { '0' + digit, '\0' };
            print(c);
            started = 1;
        }
        div /= 10;
    }

    print("K");
}

void mem(const char *args, uint32_t *cwd) {
    ram_used();
    print(" / ");
    ram_allocated();
    print("\n");
}
