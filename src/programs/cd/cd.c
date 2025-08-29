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
#include "ramdisk.h"
#include "vga.h"
#include "string.h"

void cd(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: cd <dirname>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    const char *dirname = args;
    if (kstrcmp(dirname, "..") == 0) {
        if (current_dir_inode_no != 0) {
            ramdisk_inode_t *cur_dir = ramdisk_iget(current_dir_inode_no);
            if (cur_dir) current_dir_inode_no = cur_dir->parent_inode_no;
        }
        return;
    }
    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
    if (!dir) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get current directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    ramdisk_inode_t *new_dir = ramdisk_find_inode_by_name(dir, dirname);
    if (!new_dir || new_dir->type != RAMDISK_INODE_TYPE_DIR) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Directory not found\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    current_dir_inode_no = new_dir->inode_no;
}