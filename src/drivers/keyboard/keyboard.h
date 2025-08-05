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

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KEY_NULL         ((int)0x00)
#define KEY_BACKSPACE    ((int)'\b')
#define KEY_TAB          ((int)'\t')
#define KEY_ENTER        ((int)'\n')
#define KEY_ESCAPE       ((int)0x1B)
#define KEY_SPACE        ((int)' ')  
#define KEY_DELETE       ((int)0x86)
#define KEY_LEFT         ((int)0x80)
#define KEY_RIGHT        ((int)0x81)
#define KEY_UP           ((int)0x82)
#define KEY_DOWN         ((int)0x83)
#define KEY_HOME         ((int)0x84)
#define KEY_END          ((int)0x85)
#define KEY_PAGE_UP      ((int)0x87)
#define KEY_PAGE_DOWN    ((int)0x88)
#define KEY_INSERT       ((int)0x89)
#define KEY_F1           ((int)0x90)
#define KEY_F2           ((int)0x91)
#define KEY_F3           ((int)0x92)
#define KEY_F4           ((int)0x93)
#define KEY_F5           ((int)0x94)
#define KEY_F6           ((int)0x95)
#define KEY_F7           ((int)0x96)
#define KEY_F8           ((int)0x97)
#define KEY_F9           ((int)0x98)
#define KEY_F10          ((int)0x99)
#define KEY_F11          ((int)0x9A)
#define KEY_F12          ((int)0x9B)
#define KEY_SHIFT        ((int)0xA0)
#define KEY_CTRL         ((int)0xA1)
#define KEY_ALT          ((int)0xA2)
#define KEY_CAPS_LOCK    ((int)0xA3)
#define KEY_NUM_LOCK     ((int)0xA4)
#define KEY_SCROLL_LOCK  ((int)0xA5)
#define KEY_PRINT_SCREEN ((int)0xA6)
#define KEY_PAUSE        ((int)0xA7)
#define KEY_MENU         ((int)0xA8)
#define KEY_WIN          ((int)0xA9)

int keyboard_getchar();

#endif
