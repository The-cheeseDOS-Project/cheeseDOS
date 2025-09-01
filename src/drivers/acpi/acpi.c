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

#include "stdint.h"
#include "stddef.h"
#include "vga.h"
#include "serial.h"
#include "io.h"

static uint16_t pm1a_cnt = 0;

void shutdown() {
    uint16_t ports[] = { 0xB004, 0x604, 0x4004, 0x600 };
    uint16_t values[] = { 0x2000, 0x3400, 0x34 };

    print("Shutting down...\n");
    sprint("\nShutting down target...\n\n");

    for (size_t i = 0; i < sizeof(ports) / sizeof(ports[0]); i++) {
        for (size_t j = 0; j < sizeof(values) / sizeof(values[0]); j++) {
            outw(ports[i], values[j]);
        }
    }

    if (pm1a_cnt) {
        outw(pm1a_cnt, 5 << 10 | 1 << 13);
    }
}

void reboot() {
    print("Rebooting...");
    sprint("\nRebooting target...\n\n");

    outw(0x64, 0xFE);
}
