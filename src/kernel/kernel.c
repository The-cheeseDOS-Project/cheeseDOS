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
#include "version.h"
#include "string.h"

void kmain() {
    char buf[16];
    clear_screen();
    qprint("Loading KERNEL... OK!\n");
    sprint("cheeseDOS version ");
    sprint(_binary_src_version_version_txt_start);
    qprint("Loading ");
    itoa(RAMDISK_DATA_SIZE_BYTES, buf, 10);
    qprint(buf);
    qprint("B RAMDISK...");
    ramdisk_init();
    qprint(" OK!\nLoading SHELL...");
    shell_run();

    while (1)
        __asm__("hlt");
}
