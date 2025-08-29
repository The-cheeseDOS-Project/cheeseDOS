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
#include "vga.h"
#include "ramdisk.h"

void see(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: see <filename>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    const char *filename = args;
    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
    if (!dir) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get current directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    ramdisk_inode_t *file = ramdisk_find_inode_by_name(dir, filename);
    if (!file) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("File not found\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    if (file->type == RAMDISK_INODE_TYPE_DIR) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Cannot see directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    char buf[2048];
    int read = ramdisk_readfile(file, 0, sizeof(buf) - 1, buf);
    if (read < 0) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Error reading file\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    buf[read] = 0;

    print(buf);
    print("\n");
}