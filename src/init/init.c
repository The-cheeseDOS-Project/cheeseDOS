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
#include "serial.h"
#include "shell.h"
#include "ramdisk.h"
#include "ide.h"
#include "string.h"

__attribute__((section(".text"))) uint8_t _init_start;

extern uint8_t _init_start;
extern uint8_t _end;

uint32_t heap_ptr;

void init() {
    char buf[16];
    
    bprint("Loading cheeseDOS...\n");

    bprint("Setting Heap pointer...");
    heap_ptr = (uint32_t)&_end;

    set_text_color(COLOR_GREEN, COLOR_BLACK);
    print(" Done!\n");
    set_text_color(COLOR_WHITE, COLOR_BLACK);

    sprint(" \033[92mDone!\033[0m\n");

    bprint("Loading ");
    itoa(RAMDISK_DATA_SIZE_BYTES, buf, 10);
    bprint(buf);
    bprint("B RAM Disk...");
    
    ramdisk_init();
    
    set_text_color(COLOR_GREEN, COLOR_BLACK);
    print(" Done!\n");
    set_text_color(COLOR_WHITE, COLOR_BLACK);

    sprint(" \033[92mDone!\033[0m\n");

    bprint("Looking for IDE master... ");
    if (ide_detect()) {
        set_text_color(COLOR_GREEN, COLOR_BLACK);
        print(" Found!\n");
        set_text_color(COLOR_WHITE, COLOR_BLACK);

        sprint(" \033[92mFound!\033[0m\n");
        
        bprint("Loading IDE Drive...");
        
        if (ide_init()) {
            sprint(" \033[92mDone!\033[0m\n");
            set_text_color(COLOR_GREEN, COLOR_BLACK);
            print(" Done!\n");
            set_text_color(COLOR_WHITE, COLOR_BLACK);
        } else {
            bprint(" Failed!\n");
        }
    } else {
        bprint("Not Found!\n");
    }

    bprint("Loading Shell...");
    shell_run();
}
