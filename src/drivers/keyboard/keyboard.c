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

#include <stdint.h>
#include "vga.h"
#include "keyboard.h"

static const char scancode_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/', 0,'*', 0,' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char scancode_ascii_shift[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
    'A','S','D','F','G','H','J','K','L',':','"','~', 0,'|',
    'Z','X','C','V','B','N','M','<','>','?', 0,'*', 0,' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char numpad_ascii[128] = {
    [0x47] = '7', [0x48] = '8', [0x49] = '9',
    [0x4B] = '4', [0x4C] = '5', [0x4D] = '6',
    [0x4F] = '1', [0x50] = '2', [0x51] = '3',
    [0x52] = '0', [0x53] = '.',
    [0x4A] = '-', [0x4E] = '+', [0x37] = '*', [0x35] = '/'
};

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

int keyboard_getchar() {
    uint8_t sc;
    static int shift = 0;
    static int caps_lock = 0;
    static int num_lock = 1;

    while (1) {

        while (!(inb(0x64) & 1));
        sc = inb(0x60);

        if (sc == 0xE0) {

            while (!(inb(0x64) & 1));
            sc = inb(0x60);

            if (sc == 0x4B) return KEY_LEFT;
            if (sc == 0x4D) return KEY_RIGHT;
            if (sc == 0x48) return KEY_UP;
            if (sc == 0x50) return KEY_DOWN;
            if (sc == 0x47) return KEY_HOME;
            if (sc == 0x4F) return KEY_END;
            if (sc == 0x53) return KEY_DELETE;

            continue;
        }

        if (sc == 0x2A || sc == 0x36) {
            shift = 1;
            continue;
        }

        if (sc == 0xAA || sc == 0xB6) {
            shift = 0;
            continue;
        }

        if (sc == 0x3A) {
            caps_lock = !caps_lock;
            continue;
        }

        if (sc == 0x45) {
            num_lock = !num_lock;
            continue;
        }

        if (!(sc & 0x80)) {
            sc &= 0x7F;

            if (num_lock && numpad_ascii[sc]) {
                return numpad_ascii[sc];
            }

            if (sc < 128) {
                char c = scancode_ascii[sc];

                if (c >= 'a' && c <= 'z') {
                    if (shift ^ caps_lock) {
                        c -= 32;
                    }
                } else if (shift) {
                    c = scancode_ascii_shift[sc];
                }

                return c;
            }
        }

    }
}