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

void mve(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: mve <src> <dst>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    char src[RAMDISK_FILENAME_MAX];
    char dst[RAMDISK_FILENAME_MAX];
    int i = 0, j = 0;
    while (args[i] == ' ') i++;
    while (args[i] && args[i] != ' ' && j < RAMDISK_FILENAME_MAX - 1) {
        src[j++] = args[i++];
    }
    src[j] = '\0';
    while (args[i] == ' ') i++;
    j = 0;
    while (args[i] && j < RAMDISK_FILENAME_MAX - 1) {
        dst[j++] = args[i++];
    }
    dst[j] = '\0';

    if (src[0] == '\0' || dst[0] == '\0') {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: mve <src> <dst>\n");
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
    size_t len = kstrlen(dst);
    if (len >= RAMDISK_FILENAME_MAX) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Destination name too long\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    for (size_t k = 0; k < RAMDISK_FILENAME_MAX; ++k) src_inode->name[k] = 0;
    for (size_t k = 0; k < len; ++k) src_inode->name[k] = dst[k];
    src_inode->name[len] = 0;
}