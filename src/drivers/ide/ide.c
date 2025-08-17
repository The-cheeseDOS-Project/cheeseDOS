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
#include "serial.h"
#include "vga.h"

#define IDE_DATA       0x1F0
#define IDE_ERROR      0x1F1
#define IDE_SECCOUNT   0x1F2
#define IDE_LBA_LOW    0x1F3
#define IDE_LBA_MID    0x1F4
#define IDE_LBA_HIGH   0x1F5
#define IDE_DRIVE_HEAD 0x1F6
#define IDE_COMMAND    0x1F7
#define IDE_STATUS     0x1F7

#define IDE_CMD_IDENTIFY 0xEC
#define IDE_DRIVE_MASTER 0xA0

static uint16_t identify_buffer[256];

static uint8_t ide_status = 0;

void ide_wait_ready() {
    while (inb(IDE_STATUS) & 0x80);
    while (!(inb(IDE_STATUS) & 0x08));
}

int ide_init() {
    outb(IDE_DRIVE_HEAD, IDE_DRIVE_MASTER);
    outb(IDE_COMMAND, IDE_CMD_IDENTIFY);

    ide_status = inb(IDE_STATUS);

    if (ide_status == 0) {
        qprint("NOT FOUND!\n");
        return 0;
    }

    ide_wait_ready();

    for (int i = 0; i < 256; i++) {
        identify_buffer[i] = inw(IDE_DATA);
    }

    qprint("FOUND!\n");
    return 1;
}

int print_drive_present(void) {
    if (ide_status == 0) {
        print("False\n");
        return 0;
    }
    print("True\n");
    return 1;
}