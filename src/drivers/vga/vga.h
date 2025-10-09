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

#ifndef VGA_H
#define VGA_H

#define COLOR_BLACK         0x0
#define COLOR_DARKBLUE      0x1
#define COLOR_DARKGREEN     0x2
#define COLOR_DARKCYAN      0x3
#define COLOR_DARKRED       0x4
#define COLOR_PURPLE        0x5
#define COLOR_BROWN         0x6
#define COLOR_LIGHT_GREY    0x7
#define COLOR_DARK_GREY     0x8
#define COLOR_BLUE          0x9
#define COLOR_GREEN         0xA
#define COLOR_CYAN          0xB
#define COLOR_RED           0xC
#define COLOR_MAGENTA       0xD
#define COLOR_YELLOW        0xE
#define COLOR_WHITE         0xF

#define VGA_COLOR(fg, bg) ((bg << 4) | (fg & 0x0F))

extern unsigned char default_text_fg_color;
extern unsigned char default_text_bg_color;

void putchar(char c);
void print(const char *str);
void clear_screen(void);
void backspace(void);
void set_cursor_pos(unsigned short pos);
unsigned short get_cursor(void);
void set_text_color(unsigned char fg, unsigned char bg);
unsigned char get_screen_width(void);
unsigned char get_screen_height(void);
void clear_chars(unsigned short start_pos, unsigned short count);
void putchar(char ch);
void set_cursor(unsigned char row, unsigned char col);
void get_cursor(unsigned char *row, unsigned char *col);
void move_cursor(unsigned char row, unsigned char col);
void hide_cursor(int hide);
void uprint(unsigned int num);
void disable_scroll(int disable);

#endif
