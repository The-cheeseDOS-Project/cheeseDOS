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
#include "font.h"

extern FontChar font[]; 
extern const int font_count;  

int cursor_x = 0;
int cursor_y = 0;
const int CHAR_WIDTH = 4;
const int CHAR_HEIGHT = 6;
const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 200;

// old stuff:
#define VGA_MEMORY ((uint16_t*)0xB8000)
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

static uint8_t vga_cursor_x = 0;
static uint8_t vga_cursor_y = 0;
static uint8_t current_fg = COLOR_WHITE;
static uint8_t current_bg = COLOR_BLACK;
static bool vga_scrolling_enabled = true;
static bool vga_cursor_hidden = false;
static uint8_t saved_cursor_start = 0x00;
static uint8_t saved_cursor_end = 0x0F;

uint8_t default_text_fg_color = COLOR_WHITE;
uint8_t default_text_bg_color = COLOR_BLACK;
// old stuff end

static uint8_t get_vga_color(void) {
  //    return VGA_COLOR(current_fg, current_bg);
}

void set_cursor(uint16_t position) {
  //if (!vga_cursor_hidden) {
  //        outb(0x3D4, 0x0A); outb(0x3D5, saved_cursor_start);
  //        outb(0x3D4, 0x0B); outb(0x3D5, saved_cursor_end);
  //    }
  //    outb(0x3D4, 0x0F); outb(0x3D5, position & 0xFF);
  //    outb(0x3D4, 0x0E); outb(0x3D5, (position >> 8) & 0xFF);
}

void vga_hide_cursor(int hide) {
  //    if (hide) {
  //        if (vga_cursor_hidden) return;
  //        outb(0x3D4, 0x0A);
  //        saved_cursor_start = inb(0x3D5);
  //        outb(0x3D4, 0x0B);
  //        saved_cursor_end = inb(0x3D5);
  //
  //        outb(0x3D4, 0x0A); outb(0x3D5, 0x20);
  //        vga_cursor_hidden = true;
  //    } else {
  //        if (!vga_cursor_hidden) return;
  //        outb(0x3D4, 0x0A); outb(0x3D5, saved_cursor_start);
  //        outb(0x3D4, 0x0B); outb(0x3D5, saved_cursor_end);
  //        vga_cursor_hidden = false;
  //    }
}

void scroll_screen(void) {
  //    for (uint8_t row = 1; row < SCREEN_HEIGHT; row++) {
  //        for (uint8_t col = 0; col < SCREEN_WIDTH; col++) {
  //            VGA_MEMORY[(row - 1) * SCREEN_WIDTH + col] =
  //                VGA_MEMORY[row * SCREEN_WIDTH + col];
  //        }
  //    }
  //    uint8_t color_byte = get_vga_color();
  //    for (uint8_t col = 0; col < SCREEN_WIDTH; col++) {
  //        VGA_MEMORY[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + col] =
  //            ' ' | (color_byte << 8);
  //    }
}

void vga_putchar(char c) {
  //    uint8_t color_byte = get_vga_color();
  //
  //    if (c == '\n') {
  //        vga_cursor_x = 0;
  //        vga_cursor_y++;
  //    } else if (c == '\b') {
  //        if (vga_cursor_x > 0) {
  //            vga_cursor_x--;
  //        } else if (vga_cursor_y > 0) {
  //            vga_cursor_y--;
  //            vga_cursor_x = SCREEN_WIDTH - 1;
  //        }
  //        VGA_MEMORY[vga_cursor_y * SCREEN_WIDTH + vga_cursor_x] = ' ' | (color_byte << 8);
  //    } else {
  //        VGA_MEMORY[vga_cursor_y * SCREEN_WIDTH + vga_cursor_x] = c | (color_byte << 8);
  //        vga_cursor_x++;
  //    }
  //
  //    if (vga_cursor_x >= SCREEN_WIDTH) {
  //        vga_cursor_x = 0;
  //        vga_cursor_y++;
  //    }
  //    if (vga_cursor_y >= SCREEN_HEIGHT) {
  //        if (vga_scrolling_enabled) {
  //            scroll_screen();
  //            vga_cursor_y = SCREEN_HEIGHT - 1;
  //        } else {
  //            vga_cursor_y = SCREEN_HEIGHT - 1;
  //            vga_cursor_x = 0;
  //        }
  //    }
  //    set_cursor(vga_cursor_y * SCREEN_WIDTH + vga_cursor_x);
}

void print(const char *str) {
  //    while (*str) vga_putchar(*str++);
}

//void clear_screen(void) {
  //    uint16_t blank = ' ' | (get_vga_color() << 8);
  //    uint16_t *video = (uint16_t *)VGA_MEMORY;
  //    
  //    for (int i = 0; i < SCREEN_SIZE; i++) {
  //        video[i] = blank;
  //    }
  //    
  //    vga_cursor_x = 0;
  //    vga_cursor_y = 0;
  //    set_cursor(0);
//}

void vga_set_cursor(uint8_t row, uint8_t col) {
  //    vga_cursor_x = col;
  //    vga_cursor_y = row;
  //    uint16_t pos = row * SCREEN_WIDTH + col;
  //    outb(0x3D4, 0x0F); outb(0x3D5, pos & 0xFF);
  //    outb(0x3D4, 0x0E); outb(0x3D5, (pos >> 8) & 0xFF);
}

uint16_t get_cursor(void) {
  //    return vga_cursor_y * SCREEN_WIDTH + vga_cursor_x;
}

void vga_get_cursor(uint8_t *row, uint8_t *col) {
  //    outb(0x3D4, 0x0F);
  //    uint8_t low = inb(0x3D5);
  //    outb(0x3D4, 0x0E);
  //    uint8_t high = inb(0x3D5);
  //    uint16_t pos = (high << 8) | low;
  //    *row = pos / SCREEN_WIDTH;
  //    *col = pos % SCREEN_WIDTH;
}

void set_cursor_pos(uint16_t pos) {
  //    if (pos >= SCREEN_SIZE) pos = SCREEN_SIZE - 1;
  //    vga_cursor_x = pos % SCREEN_WIDTH;
  //    vga_cursor_y = pos / SCREEN_WIDTH;
  //    set_cursor(pos);
}

void set_text_color(uint8_t fg, uint8_t bg) {
  //    current_fg = fg & 0x0F;
  //    current_bg = bg & 0x0F;
}

uint8_t get_screen_width(void) {
  //   return SCREEN_WIDTH;
}

uint8_t get_screen_height(void) {
  //    return SCREEN_HEIGHT;
}

void vga_clear_chars(uint16_t start_pos, uint16_t count) {
  //    uint8_t color_byte = get_vga_color();
  //    uint16_t end_pos = start_pos + count;
  //    if (end_pos > SCREEN_SIZE) end_pos = SCREEN_SIZE;
  //    for (uint16_t i = start_pos; i < end_pos; i++) {
  //        VGA_MEMORY[i] = ' ' | (color_byte << 8);
  //    }
}

void vga_move_cursor(uint8_t row, uint8_t col) {
  //    vga_cursor_x = col;
  //    vga_cursor_y = row;
  //    set_cursor(row * SCREEN_WIDTH + col);
}

void print_uint(uint32_t num) {
  //    char buf[12];
  //    uint8_t i = 0;
  //    if (num == 0) {
  //        vga_putchar('0');
  //        return;
  //    }
  //    while (num > 0) {
  //        buf[i++] = (num % 10) + '0';
  //        num /= 10;
  //    }
  //    while (i--) vga_putchar(buf[i]);
}

void vga_disable_scroll(int disable) {
  //    vga_scrolling_enabled = !disable;
}

void set_palette(int index, int r3, int g3, int b3) {
    int r = r3 * 9;
    int g = g3 * 9;
    int b = b3 * 9;

    outb(0x3C8, 0);  
    for (int i = 0; i < 256; i++) {
        outb(0x3C9, 0);  
        outb(0x3C9, 0);  
        outb(0x3C9, 0);  
    }

    outb(0x3C8, index);  
    outb(0x3C9, r);      
    outb(0x3C9, g);      
    outb(0x3C9, b);      
}

void graphics_mode(void) {
    outb(0x3C2, 0x63);  

    outb(0x3C4, 0x00); outb(0x3C5, 0x01);  
    outb(0x3C4, 0x01); outb(0x3C5, 0x01);  
    outb(0x3C4, 0x02); outb(0x3C5, 0x0F);  
    outb(0x3C4, 0x03); outb(0x3C5, 0x00);  
    outb(0x3C4, 0x04); outb(0x3C5, 0x0E);  
    outb(0x3C4, 0x00); outb(0x3C5, 0x03);  

    outb(0x3D4, 0x11); outb(0x3D5, 0x00);  

    outb(0x3D4, 0x00); outb(0x3D5, 0x5F);  
    outb(0x3D4, 0x01); outb(0x3D5, 0x4F);  
    outb(0x3D4, 0x02); outb(0x3D5, 0x50);  
    outb(0x3D4, 0x03); outb(0x3D5, 0x82);  
    outb(0x3D4, 0x04); outb(0x3D5, 0x54);  
    outb(0x3D4, 0x05); outb(0x3D5, 0x80);  
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
    outb(0x3CE, 0x05); outb(0x3CF, 0x40);  
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

    volatile uint8_t *vga = (uint8_t *)0xA0000;
    for (uint32_t i = 0; i < 64000; i++) {  
        vga[i] = 0x00;
    }

    set_palette(0, 0,0,0);
    set_palette(1, 0,0,7);
    set_palette(2, 0,7,0);
    set_palette(3, 0,7,7);
    set_palette(4, 7,0,0);
    set_palette(5, 7,0,7);
    set_palette(6, 7,7,0);
    set_palette(7, 7,7,7);
}

void put_pixel(int x, int y, int color) {
    if (x >= 320 || y >= 200) return;
    
    unsigned int offset = y * 320 + x;
    
    volatile unsigned char *vga = (unsigned char *)0xA0000;
    vga[offset] = (unsigned char)color;
}

FontChar* find_glyph(char c) {
    for (int i = 0; i < font_count; i++) {
        if (font[i].c == c) return &font[i];
    }
    return 0; 
}

void draw_char(int x, int y, char c, int color) {
    FontChar* glyph = find_glyph(c);
    if (!glyph) return; 

    for (int row = 0; row < CHAR_HEIGHT; row++) {
        uint8_t bits = glyph->bitmap[row];
        for (int col = 0; col < CHAR_WIDTH; col++) {
	    if (bits & (1 << (7 - col))) {
                put_pixel(x + col, y + row, color);
            }
        }
    }
}

void scroll(void) {
    volatile unsigned char *vga = (unsigned char *)0xB8000; 
    int row_size = SCREEN_WIDTH;

    for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i++) {
        vga[i] = vga[i + row_size];
    }

    for (int i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH;
         i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
        vga[i] = 0x00;
    }
}

void gprint(const char *text) {
    while (*text) {
        if (*text == '\n') {
            cursor_x = 0;
            cursor_y += CHAR_HEIGHT;
            if (cursor_y + CHAR_HEIGHT > SCREEN_HEIGHT) {
                scroll();
                cursor_y -= CHAR_HEIGHT;
            }
        } else {
            draw_char(cursor_x, cursor_y, *text, 7); 
            cursor_x += CHAR_WIDTH;
            if (cursor_x + CHAR_WIDTH > SCREEN_WIDTH) {
                cursor_x = 0;
                cursor_y += CHAR_HEIGHT;
                if (cursor_y + CHAR_HEIGHT > SCREEN_HEIGHT) {
                    scroll();
                    cursor_y -= CHAR_HEIGHT;
                }
            }
        }
        text++;
    }
}
 
void clear_screen(void) {
    volatile unsigned char *vga = (unsigned char *)0xA0000;
    for (unsigned int i = 0; i < 320 * 200; i++) {
        vga[i] = 0;
    }
}
