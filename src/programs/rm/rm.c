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

void rm(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: rm <filename>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    int res = ramdisk_remove_file(current_dir_inode_no, args);
    if (res == 0) {
    } else {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to remove file\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
    }
}