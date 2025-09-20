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

void cpy(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: cpy <src> <dst>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    char src[RAMDISK_FILENAME_MAX];
    char dst[RAMDISK_FILENAME_MAX];
    int i = 0, j = 0;
    while (args[i] == ' ') i++;
    while (args[i] && args[i] != ' ' && j < RAMDISK_FILENAME_MAX - 1) src[j++] = args[i++];
    src[j] = '\0';

    while (args[i] == ' ') i++;
    j = 0;
    while (args[i] && j < RAMDISK_FILENAME_MAX - 1) dst[j++] = args[i++];
    dst[j] = '\0';

    if (src[0] == '\0' || dst[0] == '\0') {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: cpy <src> <dst>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
    if (!dir) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get current directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    ramdisk_inode_t *src_inode = ramdisk_find_inode_by_name(dir, src);
    if (!src_inode) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Source not found\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    ramdisk_inode_t *dst_inode = ramdisk_find_inode_by_name(dir, dst);
    if (dst_inode) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Destination already exists\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    int create_result = (src_inode->type == RAMDISK_INODE_TYPE_FILE)
        ? ramdisk_create_file(current_dir_inode_no, dst)
        : ramdisk_create_dir(current_dir_inode_no, dst);

    if (create_result != 0) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to create destination\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    dst_inode = ramdisk_find_inode_by_name(dir, dst);
    if (!dst_inode) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get destination inode\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    copy_inode(src_inode, dst_inode);
}
