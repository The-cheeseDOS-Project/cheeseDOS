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
#include "io.h"
#include "vga.h"
#include "serial.h"

void reboot(void) {
    clear_screen();
    set_text_color(COLOR_WHITE, COLOR_BLACK);
    qprint("Rebooting...\n");
    sprint("Wait for keyboard controller...\n");
    while (inb(0x64) & 0x02);
    sprint("Sent reset command...\n");
    outb(0x64, 0xFE);
}

void shutdown(void) {
    set_text_color(COLOR_WHITE, COLOR_BLACK);
    print("Shutting down...\n");
    sprint("\n");
    set_text_color(COLOR_YELLOW, COLOR_BLACK);
    clear_screen();
    sprint("Halting CPU...\n");
    print("It is now safe to turn off your computer.\n");
    vga_move_cursor(80, 25);
    __asm__ volatile (
        "cli\n\t"
        "hlt\n\t"
        "jmp .-1"
    );
}