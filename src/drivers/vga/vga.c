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
#include "stdbool.h"
#include "vga.h"
#include "keyboard.h"
#include "io.h"

#define VGA_MEMORY ((uint16_t*)0xB8000)
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

static uint8_t vga_cursor_x = 0;
static uint8_t vga_cursor_y = 0;
static uint8_t current_fg = COLOR_WHITE;
static uint8_t current_bg = COLOR_BLACK;
static bool vga_scrolling_enabled = true;

static uint8_t get_vga_color(void) {
    return VGA_COLOR(current_fg, current_bg);
}

void set_cursor(uint16_t position) {
    outb(0x3D4, 0x0A); outb(0x3D5, 0x00);
    outb(0x3D4, 0x0B); outb(0x3D5, 0x0F);
    outb(0x3D4, 0x0F); outb(0x3D5, position & 0xFF);
    outb(0x3D4, 0x0E); outb(0x3D5, (position >> 8) & 0xFF);
}

void scroll_screen(void) {
    for (uint8_t row = 1; row < SCREEN_HEIGHT; row++) {
        for (uint8_t col = 0; col < SCREEN_WIDTH; col++) {
            VGA_MEMORY[(row - 1) * SCREEN_WIDTH + col] =
                VGA_MEMORY[row * SCREEN_WIDTH + col];
        }
    }
    uint8_t color_byte = get_vga_color();
    for (uint8_t col = 0; col < SCREEN_WIDTH; col++) {
        VGA_MEMORY[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + col] =
            ' ' | (color_byte << 8);
    }
}

void vga_putchar(char c) {
    uint8_t color_byte = get_vga_color();

    if (c == '\n') {
        vga_cursor_x = 0;
        vga_cursor_y++;
    } else if (c == '\b') {
        if (vga_cursor_x > 0) {
            vga_cursor_x--;
        } else if (vga_cursor_y > 0) {
            vga_cursor_y--;
            vga_cursor_x = SCREEN_WIDTH - 1;
        }
        VGA_MEMORY[vga_cursor_y * SCREEN_WIDTH + vga_cursor_x] = ' ' | (color_byte << 8);
    } else {
        VGA_MEMORY[vga_cursor_y * SCREEN_WIDTH + vga_cursor_x] = c | (color_byte << 8);
        vga_cursor_x++;
    }

    if (vga_cursor_x >= SCREEN_WIDTH) {
        vga_cursor_x = 0;
        vga_cursor_y++;
    }
    if (vga_cursor_y >= SCREEN_HEIGHT) {
        if (vga_scrolling_enabled) {
            scroll_screen();
            vga_cursor_y = SCREEN_HEIGHT - 1;
        } else {
            vga_cursor_y = SCREEN_HEIGHT - 1;
            vga_cursor_x = 0;
        }
    }
    set_cursor(vga_cursor_y * SCREEN_WIDTH + vga_cursor_x);
}

void print(const char *str) {
    while (*str) vga_putchar(*str++);
}

void clear_screen(void) {
    uint8_t color_byte = get_vga_color();
    for (uint16_t i = 0; i < SCREEN_SIZE; i++) {
        VGA_MEMORY[i] = ' ' | (color_byte << 8);
    }
    vga_cursor_x = 0;
    vga_cursor_y = 0;
    set_cursor(0);
}

void vga_set_cursor(uint8_t row, uint8_t col) {
    vga_cursor_x = col;
    vga_cursor_y = row;
    uint16_t pos = row * SCREEN_WIDTH + col;
    outb(0x3D4, 0x0F); outb(0x3D5, pos & 0xFF);
    outb(0x3D4, 0x0E); outb(0x3D5, (pos >> 8) & 0xFF);
}

uint16_t get_cursor(void) {
    return vga_cursor_y * SCREEN_WIDTH + vga_cursor_x;
}

void vga_get_cursor(uint8_t *row, uint8_t *col) {
    outb(0x3D4, 0x0F);
    uint8_t low = inb(0x3D5);
    outb(0x3D4, 0x0E);
    uint8_t high = inb(0x3D5);
    uint16_t pos = (high << 8) | low;
    *row = pos / SCREEN_WIDTH;
    *col = pos % SCREEN_WIDTH;
}

void set_cursor_pos(uint16_t pos) {
    if (pos >= SCREEN_SIZE) pos = SCREEN_SIZE - 1;
    vga_cursor_x = pos % SCREEN_WIDTH;
    vga_cursor_y = pos / SCREEN_WIDTH;
    set_cursor(pos);
}

void set_text_color(uint8_t fg, uint8_t bg) {
    current_fg = fg & 0x0F;
    current_bg = bg & 0x0F;
}

uint8_t get_screen_width(void) { return SCREEN_WIDTH; }
uint8_t get_screen_height(void) { return SCREEN_HEIGHT; }

void vga_clear_chars(uint16_t start_pos, uint16_t count) {
    uint8_t color_byte = get_vga_color();
    uint16_t end_pos = start_pos + count;
    if (end_pos > SCREEN_SIZE) end_pos = SCREEN_SIZE;
    for (uint16_t i = start_pos; i < end_pos; i++) {
        VGA_MEMORY[i] = ' ' | (color_byte << 8);
    }
}

void vga_move_cursor(uint8_t row, uint8_t col) {
    vga_cursor_x = col;
    vga_cursor_y = row;
    set_cursor(row * SCREEN_WIDTH + col);
}

void print_uint(uint32_t num) {
    char buf[12];
    uint8_t i = 0;
    if (num == 0) {
        vga_putchar('0');
        return;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i--) vga_putchar(buf[i]);
}

void vga_disable_scroll(bool disable) {
    vga_scrolling_enabled = !disable;
}
