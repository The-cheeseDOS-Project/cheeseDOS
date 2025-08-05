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
#include "vga.h"
#include "keyboard.h"

#define VGA_MEMORY ((uint16_t*)0xB8000)
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

static int vga_cursor_x = 0;
static int vga_cursor_y = 0;
static uint8_t current_fg = COLOR_WHITE;
static uint8_t current_bg = COLOR_BLACK;

static uint8_t get_vga_color() {
    return VGA_COLOR(current_fg, current_bg);
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void set_cursor(int position) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x00);
    outb(0x3D4, 0x0B);
    outb(0x3D5, 0x0F);

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(position & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}

void scroll_screen() {
    for (int row = 1; row < SCREEN_HEIGHT; row++) {
        for (int col = 0; col < SCREEN_WIDTH; col++) {
            VGA_MEMORY[(row - 1) * SCREEN_WIDTH + col] =
                VGA_MEMORY[row * SCREEN_WIDTH + col];
        }
    }

    uint8_t color_byte = get_vga_color();
    for (int col = 0; col < SCREEN_WIDTH; col++) {
        VGA_MEMORY[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + col] =
            ' ' | (color_byte << 8);
    }
}

void vga_putchar(char c) {
    uint8_t color_byte = get_vga_color();
    int current_linear_pos;

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
        scroll_screen();
        vga_cursor_y = SCREEN_HEIGHT - 1;
    }

    current_linear_pos = vga_cursor_y * SCREEN_WIDTH + vga_cursor_x;
    set_cursor(current_linear_pos);
}

void print(const char* str) {
    while (*str) vga_putchar(*str++);
}

void clear_screen() {
    uint8_t color_byte = get_vga_color();
    for (int i = 0; i < SCREEN_SIZE; i++) {
        VGA_MEMORY[i] = ' ' | (color_byte << 8);
    }
    vga_cursor_x = 0;
    vga_cursor_y = 0;
    set_cursor(0);
}

void backspace() {
    vga_putchar('\b');
}

int get_cursor() {
    return vga_cursor_y * SCREEN_WIDTH + vga_cursor_x;
}

void set_cursor_pos(int pos) {
    if (pos < 0) pos = 0;
    if (pos >= SCREEN_SIZE) pos = SCREEN_SIZE - 1;

    vga_cursor_x = pos % SCREEN_WIDTH;
    vga_cursor_y = pos / SCREEN_WIDTH;

    set_cursor(pos);
}

void set_text_color(uint8_t fg, uint8_t bg) {
    current_fg = fg & 0x0F;
    current_bg = bg & 0x0F;
}

int get_screen_width() {
    return SCREEN_WIDTH;
}

int get_screen_height() {
    return SCREEN_HEIGHT;
}

void vga_clear_chars(int start_pos, int count) {
    uint8_t color_byte = get_vga_color();
    int end_pos = start_pos + count;
    if (end_pos > SCREEN_SIZE) end_pos = SCREEN_SIZE;

    for (int i = start_pos; i < end_pos; i++) {
        VGA_MEMORY[i] = ' ' | (color_byte << 8);
    }
}