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

#include "string.h"
#include "vga.h"
#include "serial.h"
#include "shell.h"
#include "ramdisk.h"
#include "ide.h"

void init() {
    char buf[16];

    bprint("Loading cheeseDOS...\n");

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
            set_text_color(COLOR_RED, COLOR_BLACK);
            print(" Failed!\n");
            set_text_color(COLOR_WHITE, COLOR_BLACK);

            sprint(" \033[91mFailed!\033[0m\n");
        }
    } else {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Not Found!\n");
        set_text_color(COLOR_WHITE, COLOR_BLACK);

        sprint("\033[91mNot Found!\033[0m\n");
    }

    bprint("Loading Shell...");
    shell_run();
}
