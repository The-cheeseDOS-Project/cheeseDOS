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
#include "string.h"

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
    return ide_status != 0;
}

static char disk_buffer[512];
static int disk_offset = 0;

static void int_to_str(int value, char *out) {
    char tmp[16];
    int i = 0, j;
    if (value == 0) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }
    while (value > 0 && i < 15) {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    }
    for (j = 0; j < i; j++) {
        out[j] = tmp[i - j - 1];
    }
    out[j] = '\0';
}

static void write(const char *buffer, int cylinder, int head, int sector) {
    outb(0x1F2, 1);
    outb(0x1F3, sector);
    outb(0x1F4, cylinder & 0xFF);
    outb(0x1F5, (cylinder >> 8) & 0xFF);
    outb(0x1F6, 0xA0 | (head & 0x0F));
    outb(0x1F7, 0x30);
    ide_wait_ready();
    for (int i = 0; i < 256; i++) {
        outw(0x1F0, ((uint16_t*)buffer)[i]);
    }
    ide_wait_ready();
}

void write_file(const char *path, const char *file, const char *data) {
    char entry[256];
    char size_str[16];
    memset(entry, 0, sizeof(entry));

    int size = kstrlen(data);
    int_to_str(size, size_str);

    if (path && path[0] != '\0') {
        kstrcpy(entry, path);
        kstrcat(entry, " ");
    } else {
        kstrcpy(entry, "");
    }

    kstrcat(entry, file);
    kstrcat(entry, " size=");
    kstrcat(entry, size_str);
    kstrcat(entry, " data=");
    kstrcat(entry, data);
    kstrcat(entry, " ");

    int written = kstrlen(entry);

    if ((unsigned int)(disk_offset + written) >= sizeof(disk_buffer)) {
        write(disk_buffer, 0, 0, 1);
        disk_offset = 0;
        memset(disk_buffer, 0, sizeof(disk_buffer));
    }

    memcpy(disk_buffer + disk_offset, entry, written);
    disk_offset += written;
    write(disk_buffer, 0, 0, 1);
}
