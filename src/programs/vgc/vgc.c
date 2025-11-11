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

#include "vga.h"
#include "timer.h"
#include "cpu.h"

void vgc(const char** unused) {
    (void)unused;

    graphics_mode();

    int cx = 160;
    int cy = 100;
    int radius = 50;
    int color = 1;

    int x = 0;
    int y = radius;
    int d = 1 - radius;

    while (x <= y) {
        for (int i = cx - x; i <= cx + x; i++) {
            put_pixel(i, cy + y, color);
            put_pixel(i, cy - y, color);
        }
        for (int i = cx - y; i <= cx + y; i++) {
            put_pixel(i, cy + x, color);
            put_pixel(i, cy - x, color);
        }

        x++;
        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
    }

    halt();
}
