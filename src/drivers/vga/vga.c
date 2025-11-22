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

#include "io.h"
#include "stdint.h"
#include "font.h"

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define CHAR_WIDTH    8
#define CHAR_HEIGHT   8

int cursor_x = 0;
int cursor_y = 0;

void graphics_mode(void) {
    outb(0x3C2, 0xE3);

    outb(0x3C4, 0x00); outb(0x3C5, 0x03);
    outb(0x3C4, 0x01); outb(0x3C5, 0x01);
    outb(0x3C4, 0x02); outb(0x3C5, 0x0F);
    outb(0x3C4, 0x03); outb(0x3C5, 0x00);
    outb(0x3C4, 0x04); outb(0x3C5, 0x0E);

    outb(0x3D4, 0x11); outb(0x3D5, 0x00);
    outb(0x3D4, 0x00); outb(0x3D5, 0x5F);
    outb(0x3D4, 0x01); outb(0x3D5, 0x4F);
    outb(0x3D4, 0x02); outb(0x3D5, 0x50);
    outb(0x3D4, 0x03); outb(0x3D5, 0x82);
    outb(0x3D4, 0x04); outb(0x3D5, 0x55);
    outb(0x3D4, 0x05); outb(0x3D5, 0x81);
    outb(0x3D4, 0x06); outb(0x3D5, 0xBF);
    outb(0x3D4, 0x07); outb(0x3D5, 0x1F);
    outb(0x3D4, 0x08); outb(0x3D5, 0x00);
    outb(0x3D4, 0x09); outb(0x3D5, 0x41);
    outb(0x3D4, 0x10); outb(0x3D5, 0x9C);
    outb(0x3D4, 0x11); outb(0x3D5, 0x8E);
    outb(0x3D4, 0x12); outb(0x3D5, 0x8F);
    outb(0x3D4, 0x13); outb(0x3D5, 0x28);
    outb(0x3D4, 0x14); outb(0x3D5, 0x40);
    outb(0x3D4, 0x15); outb(0x3D5, 0x96);
    outb(0x3D4, 0x16); outb(0x3D5, 0xB9);
    outb(0x3D4, 0x17); outb(0x3D5, 0xA3);

    outb(0x3CE, 0x00); outb(0x3CF, 0x00);
    outb(0x3CE, 0x01); outb(0x3CF, 0x00);
    outb(0x3CE, 0x02); outb(0x3CF, 0x00);
    outb(0x3CE, 0x03); outb(0x3CF, 0x00);
    outb(0x3CE, 0x04); outb(0x3CF, 0x00);
    outb(0x3CE, 0x05); outb(0x3CF, 0x00);
    outb(0x3CE, 0x06); outb(0x3CF, 0x05);
    outb(0x3CE, 0x07); outb(0x3CF, 0x0F);
    outb(0x3CE, 0x08); outb(0x3CF, 0xFF);

    inb(0x3DA);
    for (int i = 0; i < 16; i++) {
        outb(0x3C0, i);
        outb(0x3C0, i);
    }
    outb(0x3C0, 0x10); outb(0x3C0, 0x41);
    outb(0x3C0, 0x11); outb(0x3C0, 0x00);
    outb(0x3C0, 0x12); outb(0x3C0, 0x0F);
    outb(0x3C0, 0x13); outb(0x3C0, 0x00);
    outb(0x3C0, 0x14); outb(0x3C0, 0x00);
    outb(0x3C0, 0x20);

    volatile uint8_t *vga = (uint8_t*)0xA0000;
    for (uint32_t i = 0; i < 38400; i++) vga[i] = 0x00;
}

void put_pixel(int x, int y, int color) {
    if (x < 0 || x >= 640 || y < 0 || y >= 480) return;
    uint16_t offset = (y * 80) + (x >> 3);
    uint8_t mask = 0x80 >> (x & 7);
    outb(0x3CE, 0x08); outb(0x3CF, mask);
    outb(0x3C4, 0x02); outb(0x3C5, 1 << (color & 3));
    volatile uint8_t *vga = (uint8_t*)0xA0000;
    vga[offset] = 0xFF;
}

void draw_char(int x, int y, char c, int color) {
    const unsigned char *glyph = 0;
    for (unsigned int i = 0; i < sizeof(termfont)/10; i++) {
        if (termfont[i*10] == c) {
            glyph = &termfont[i*10+2];
            break;
        }
    }
    if (!glyph) return;

    for (int row = 0; row < CHAR_HEIGHT; row++) {
        unsigned char bits = glyph[row];
        for (int col = 0; col < CHAR_WIDTH; col++) {
            if (bits & (1 << (7-col))) {
                put_pixel(x+col, y+row, color);
            }
        }
    }
}

void print(const char *s) {
    while (*s) {
        if (*s == '\n') {
            cursor_x = 0;
            cursor_y += CHAR_HEIGHT;
        } else {
            draw_char(cursor_x, cursor_y, *s, 15);
            cursor_x += CHAR_WIDTH;
            if (cursor_x + CHAR_WIDTH >= SCREEN_WIDTH) {
                cursor_x = 0;
                cursor_y += CHAR_HEIGHT;
            }
        }
        s++;
    }
}
