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
#include "stdint.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"
#include "beep.h"
#include "string.h"

#define NUM_KEYS 10
#define POLL_DELAY 100

typedef struct {
    char key_char;
    int frequency;
    int is_pressed;
} PianoKey;

PianoKey keyboard[NUM_KEYS] = {
    {'a', 262, 0},  
    {'s', 294, 0},  
    {'d', 330, 0},  
    {'f', 349, 0},  
    {'g', 392, 0},  
    {'h', 440, 0},  
    {'j', 494, 0},  
    {'k', 523, 0},  
    {'l', 587, 0},  
    {';', 659, 0}   
};

static uint8_t key_states[256];

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void handle_input() {
    while (inb(0x64) & 1) {
        uint8_t scancode = inb(0x60);

        if (scancode & 0x80) {

            key_states[scancode & 0x7F] = 0;
        } else {

            key_states[scancode] = 1;
        }
    }
}

static int is_key_pressed(char c) {

    switch (c) {
        case 'a': return key_states[0x1E];
        case 's': return key_states[0x1F];
        case 'd': return key_states[0x20];
        case 'f': return key_states[0x21];
        case 'g': return key_states[0x22];
        case 'h': return key_states[0x23];
        case 'j': return key_states[0x24];
        case 'k': return key_states[0x25];
        case 'l': return key_states[0x26];
        case ';': return key_states[0x27];
        default: return 0;
    }
}

static void draw_ui() {
    clear_screen();
    set_text_color(COLOR_CYAN, COLOR_BLACK);
    print("key | Press ESC to quit.\n");
    set_text_color(COLOR_WHITE, COLOR_BLACK);
    print("+-+-+-+-+-+-+-+-+-+-+\n");
    print("|A|S|D|F|G|H|J|K|L|;|\n");
    print("+-+-+-+-+-+-+-+-+-+-+\n");
    vga_set_cursor(80, 25);
}

void key(const char* *unused) {
    uint8_t old_row, old_col;
    vga_get_cursor(&old_row, &old_col);

    clear_screen();
    draw_ui();

    key_states[0x01] = 0;  

    int running = 1;
    uint32_t last_poll_time = timer_millis();

    while (running) {
        uint32_t current_time = timer_millis();

        handle_input();

        if (key_states[0x01]) {
            running = 0;
        }

        if (current_time - last_poll_time >= POLL_DELAY) {
            int any_key_pressed = 0;

            for (int i = 0; i < NUM_KEYS; i++) {
                int currently_pressed = is_key_pressed(keyboard[i].key_char);

                if (currently_pressed && !keyboard[i].is_pressed) {

                    keyboard[i].is_pressed = 1;
                    beep(keyboard[i].frequency, POLL_DELAY);
                    any_key_pressed = 1;
                } else if (!currently_pressed && keyboard[i].is_pressed) {

                    keyboard[i].is_pressed = 0;
                } else if (currently_pressed && keyboard[i].is_pressed) {

                    beep(keyboard[i].frequency, POLL_DELAY);
                    any_key_pressed = 1;
                }
            }

            draw_ui();

            last_poll_time = current_time;
        }
    }

    vga_set_cursor(old_row, old_col);
    set_text_color(COLOR_WHITE, COLOR_BLACK);
    clear_screen();
}