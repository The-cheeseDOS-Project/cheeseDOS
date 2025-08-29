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
#include "vga.h"
#include "io.h"

void box(const char*) {
    uint8_t orig_row, orig_col;
    vga_get_cursor(&orig_row, &orig_col);
    vga_set_cursor(25, 80);
    for (volatile int i = 0; i < 2000000; i++);
    while (inb(0x64) & 1) inb(0x60);

    int width = get_screen_width();
    int height = get_screen_height();
    uint16_t* vga_mem = (uint16_t*)0xB8000;

    static unsigned int seed = 1;
    seed = seed * 1103515245 + 12345;
    unsigned int rand_x = (seed / 65536) % 32768;
    seed = seed * 1103515245 + 12345;
    unsigned int rand_y = (seed / 65536) % 32768;

    const int size = 2;
    int x = size + (rand_x % (width - 2 * size));
    int y = size + (rand_y % (height - 2 * size));
    int vx = 1, vy = 1;

    uint8_t bg_colors[] = {9, 10, 11, 12, 13, 14};
    int num_colors = sizeof(bg_colors) / sizeof(bg_colors[0]);

    seed = seed * 1103515245 + 12345;
    uint8_t current_bg = bg_colors[(seed / 65536) % num_colors];
    uint8_t attr = (current_bg << 4) | 0;

    while (1) {
        if (inb(0x64) & 1) {
            inb(0x60);
            break;
        }

        clear_screen();
        x += vx;
        y += vy;
        vga_set_cursor(25, 80);

        int collided = 0;

        if (x - size <= 0 || x + size >= width) {
            vx = -vx;
            collided = 1;
            x = (x - size <= 0) ? size + 1 : width - size - 1;
        }

        if (y - size <= 0 || y + size >= height) {
            vy = -vy;
            collided = 1;
            y = (y - size <= 0) ? size + 1 : height - size - 1;
        }

        if (collided) {
            seed = seed * 1103515245 + 12345;
            uint8_t new_bg;
            do {
                new_bg = bg_colors[(seed / 65536) % num_colors];
                seed = seed * 1103515245 + 12345;
            } while (new_bg == current_bg);
            current_bg = new_bg;
            attr = (current_bg << 4) | 0;
        }

        for (int dy = -size; dy <= size; dy++) {
            for (int dx = -size - 2; dx <= size + 2; dx++) {
                int px = x + dx;
                int py = y + dy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    int pos = py * width + px;
                    vga_mem[pos] = ' ' | (attr << 8);
                }
            }
        }

        for (volatile int delay = 0; delay < 100000; delay++);
    }

    while (inb(0x64) & 1) inb(0x60);
    vga_set_cursor(orig_row, orig_col);
    clear_screen();
}