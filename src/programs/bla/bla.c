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
#include "io.h"
#include "stdint.h"
#include "random.h"
#include "timer.h"
#include "stdlib.h"

static void bresenham_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int width = get_screen_width();
    int height = get_screen_height();
    uint16_t* vga_mem = (uint16_t*)0xB8000;
    
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    int x = x0;
    int y = y0;
    
    while (1) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            int pos = y * width + x;
            vga_mem[pos] = 219 | (color << 8); 
        }
        
        if (x == x1 && y == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void bla(const char** unused) {
    (void)unused;
    uint8_t orig_row, orig_col;
    vga_get_cursor(&orig_row, &orig_col);
    vga_set_cursor(25, 80);
    
    
    while (inb(0x64) & 1) inb(0x60);
    
    int width = get_screen_width();
    int height = get_screen_height();
    
    uint8_t colors[] = {9, 10, 11, 12, 13, 14, 15};
    int num_colors = sizeof(colors) / sizeof(colors[0]);
    
    clear_screen();
    
    int first_frame = 1;
    int line_count = 0;
    
    while (1) {
        
        if (line_count >= 5) {
            clear_screen();
            line_count = 0;
        }
        
        
        int x0, y0, x1, y1;
        int distance;
        
        
        do {
            random();
            x0 = random_get() % width;
            random();
            y0 = random_get() % height;
            random();
            x1 = random_get() % width;
            random();
            y1 = random_get() % height;
            
            int dx = abs(x1 - x0);
            int dy = abs(y1 - y0);
            distance = (dx > dy) ? dx : dy; 
            
            
        } while (distance < 5 || x0 == x1 || y0 == y1);
        
        
        random();
        uint8_t color = colors[random_get() % num_colors];
        
        
        bresenham_line(x0, y0, x1, y1, color);
        line_count++;
        
        
        uint32_t start_time = timer_millis();
        while (timer_millis() - start_time < 800) {
            vga_set_cursor(25, 80);
        }
        
        
        if (first_frame) {
            first_frame = 0;
            while (inb(0x64) & 1) inb(0x60);
            continue;
        }
        
        
        if (inb(0x64) & 1) {
            inb(0x60);
            break;
        }
    }
    
    
    while (inb(0x64) & 1) inb(0x60);
    
    vga_set_cursor(orig_row, orig_col);
    clear_screen();
}
