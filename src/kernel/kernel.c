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

__attribute__((section(".text"))) uint8_t _kernel_start;

extern uint8_t _kernel_start;
extern uint8_t _end;

uint32_t heap_ptr;

void kmain() {
    char buf[16];
    
    qprint("Loading cheeseDOS...\n");

    qprint("Setting Heap pointer...");
    heap_ptr = (uint32_t)&_end;
    qprint(" Done!\n");

    qprint("Loading ");
    itoa(RAMDISK_DATA_SIZE_BYTES, buf, 10);
    qprint(buf);
    qprint("B RAM Disk...");
    ramdisk_init();
    qprint(" Done!\n");

    qprint("Looking for IDE master... ");
    if (ide_detect()) {
        qprint("Found!\nLoading IDE Drive...");
        if (ide_init()) {
            qprint(" Done!\n");
        } else {
            qprint(" Failed!\n");
        }
    } else {
        qprint("Not Found!\n");
    }

    qprint("Loading Shell...");
    shell_run();
}
