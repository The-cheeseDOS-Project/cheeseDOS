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

#include "programs.h"
#include "serial.h"

static void byte_to_hex(uint8_t byte, char* out) {
    const char* hexchars = "0123456789ABCDEF";
    out[0] = hexchars[(byte >> 4) & 0x0F];
    out[1] = hexchars[byte & 0x0F];
    out[2] = '\0';
}

void dmp(const char *unused) {
    uint8_t* ram = (uint8_t*)0x0000;
    for (uint32_t i = 0; i < 0xA0000; i++) {
        char hex[3];
        byte_to_hex(ram[i], hex);
        qprint(hex);
    }
}
