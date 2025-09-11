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

#include "vga.h"
#include "ramdisk.h"

void bit(const char* args) {
    if (!args || *args == '\0') {
        print("Usage: bit <filename>\n");
        return;
    }

    while (*args == ' ') args++;

    const char *filename = args;

    int filename_len = 0;
    while (filename[filename_len] && filename[filename_len] != ' ') {
        filename_len++;
    }

    char fname[64];
    if (filename_len >= 64) {
        print("Filename too long\n");
        return;
    }

    for (int i = 0; i < filename_len; i++) {
        fname[i] = filename[i];
    }
    fname[filename_len] = '\0';

    ramdisk_inode_t *file = ramdisk_iget_by_name(0, fname);

    if (!file) {
        print("File not found: ");
        print(fname);
        print("\n");
        return;
    }

    if (file->type == RAMDISK_INODE_TYPE_DIR) {
        print(fname);
        print(" is a directory\n");
        return;
    }

    if (file->type != RAMDISK_INODE_TYPE_FILE) {
        print(fname);
        print(" is not a regular file\n");
        return;
    }

    uint32_t size = file->size;
    if (size == 0) {
        print("0");
    } else {
        char size_str[16];
        int pos = 0;

        while (size > 0) {
            size_str[pos++] = '0' + (size % 10);
            size /= 10;
        }

        for (int i = pos - 1; i >= 0; i--) {
            char c = size_str[i];
            vga_putchar(c);
        }
    }

    print("B\n");
}