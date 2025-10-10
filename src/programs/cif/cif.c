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

#include "ramdisk.h"
#include "vga.h"
#include "string.h"
#include "stdint.h"

extern uint32_t current_dir_inode_no;
extern uint8_t default_text_fg_color;
extern uint8_t default_text_bg_color;

void cif(const char *args) {
    if (!args || args[0] == '\0') {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: cd <dirname>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    ramdisk_inode_t *cur_dir = ramdisk_iget(current_dir_inode_no);
    if (!cur_dir) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get current directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    ramdisk_inode_t *target = ramdisk_find_inode_by_name(cur_dir, args);
    if (!target || target->type != RAMDISK_INODE_TYPE_DIR) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Directory not found\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    current_dir_inode_no = target->inode_no;
}
