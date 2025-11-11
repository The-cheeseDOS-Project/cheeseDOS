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
#include "io.h"
#include "serial.h"

static void draw_circle(unsigned char* buffer, int cx, int cy, int radius, int color, int width) {
    int x = 0;
    int y = radius;
    int d = 1 - radius;

    while (x <= y) {
        for (int i = cx - x; i <= cx + x; i++) {
            if (i >= 0 && i < width && cy + y >= 0 && cy + y < 200) {
                buffer[(cy + y) * width + i] = color;
            }
            if (i >= 0 && i < width && cy - y >= 0 && cy - y < 200) {
                buffer[(cy - y) * width + i] = color;
            }
        }
        for (int i = cx - y; i <= cx + y; i++) {
            if (i >= 0 && i < width && cy + x >= 0 && cy + x < 200) {
                buffer[(cy + x) * width + i] = color;
            }
            if (i >= 0 && i < width && cy - x >= 0 && cy - x < 200) {
                buffer[(cy - x) * width + i] = color;
            }
        }

        x++;
        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
    }
}

void vgc(const char** unused) {
    (void)unused;

    graphics_mode();

    int cx = 160;
    int cy = 100;
    int radius = 50;
    int color = 1;

    int dx = 2;  
    int dy = 2;  

    int screen_width = 320;
    int screen_height = 200;

    static unsigned char back_buffer[320 * 200];

    int intro_radius = 250;
    int target_radius = 50;
    int shrink_complete = 0;

    if (inb(0x64) & 1) {
        inb(0x60);
    }
    
    while (!shrink_complete) {
        for (int i = 0; i < screen_width * screen_height; i++) {
            back_buffer[i] = 0;
        }

        draw_circle(back_buffer, cx, cy, intro_radius, color, screen_width);

        unsigned char* vga = (unsigned char*)0xA0000;
        for (int i = 0; i < screen_width * screen_height; i++) {
            vga[i] = back_buffer[i];
        }

        intro_radius -= 5;

        if (intro_radius <= target_radius) {
            intro_radius = target_radius;
            shrink_complete = 1;
        }

        delay(20);
    }

    while (1) {
        for (int i = 0; i < screen_width * screen_height; i++) {
            back_buffer[i] = 0;
        }

        draw_circle(back_buffer, cx, cy, radius, color, screen_width);

        unsigned char* vga = (unsigned char*)0xA0000;
        for (int i = 0; i < screen_width * screen_height; i++) {
            vga[i] = back_buffer[i];
        }

        cx += dx;
        cy += dy;

        int color_change = 0;

        if (cx - radius <= 0) {
            cx = radius;
            dx = -dx;
            color_change = 1;
        } else if (cx + radius >= screen_width) {
            cx = screen_width - radius;
            dx = -dx;
            color_change = 1;
        }

        if (cy - radius <= 0) {
            cy = radius;
            dy = -dy;
            color_change = 1;
        } else if (cy + radius >= screen_height) {
            cy = screen_height - radius;
            dy = -dy;
            color_change = 1;
        }

        if (color_change) {
            color = (color % 15) + 1;  
        }
	
        delay(20);
    }
}

