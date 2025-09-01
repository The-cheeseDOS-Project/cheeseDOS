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
#include "ide.h"

uint16_t identify_buffer[256];

static uint8_t ide_status = 0;

void ide_wait_ready() {
    while (inb(IDE_STATUS) & 0x80);
    while (!(inb(IDE_STATUS) & 0x08));
}

int ide_init() {
    if (!ide_detect()) {
        return 0;
    }

    ide_load_identify();
    return 1;
}

int ide_detect() {
    outb(IDE_DRIVE_HEAD, IDE_DRIVE_MASTER);
    outb(IDE_COMMAND, IDE_CMD_IDENTIFY);

    ide_status = inb(IDE_STATUS);
    if (ide_status == 0) {
        return 0;
    }

    ide_wait_ready();
    return 1;
}

void ide_load_identify() {
    for (int i = 0; i < 256; i++) {
        identify_buffer[i] = inw(IDE_DATA);
    }
}

int ide_drive_present(void) {
    if (ide_status == 0) {
        return 0;
    }
    return 1;
}
