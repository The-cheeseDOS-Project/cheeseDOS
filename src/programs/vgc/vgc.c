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
#include "stdint.h"
#include "serial.h"
#include "cpu.h"

void vgc(const char** unused) {
    (void)unused;

    sprint("\nNOTE: This is super slow and you need to reboot the computer to exit.\n");

    graphics_mode();
    
    int center_x = 320;
    int center_y = 240;
    int radius = 100;
    
    for (int y = 0; y < 480; y++) {
        for (int x = 0; x < 640; x++) {
            int dx = x - center_x;
            int dy = y - center_y;
            int dist_sq = dx * dx + dy * dy;
            int radius_sq = radius * radius;
            
            if (dist_sq <= radius_sq) {
                put_pixel(x, y, 1);
            }
        }
    }
    halt();
}
