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
#include "string.h"

void add(const char* args) {
    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
    ramdisk_inode_t *file = NULL;

    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: add <filename> <text_to_add>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    char filename[RAMDISK_FILENAME_MAX];
    const char *text_to_add = NULL;
    size_t args_len = kstrlen(args);
    const char *space_pos = NULL;

    for (size_t i = 0; i < args_len; ++i) {
        if (args[i] == ' ') {
            space_pos = &args[i];
            break;
        }
    }

    if (space_pos) {
        size_t filename_len = space_pos - args;
        if (filename_len >= RAMDISK_FILENAME_MAX) {
            set_text_color(COLOR_RED, COLOR_BLACK);
            print("Error: Filename too long.\n");
            set_text_color(default_text_fg_color, default_text_bg_color);
            return;
        }
        for (size_t i = 0; i < filename_len; ++i) {
            filename[i] = args[i];
        }
        filename[filename_len] = '\0';

        text_to_add = space_pos + 1;
        while (*text_to_add == ' ') {
            text_to_add++;
        }
        if (*text_to_add == '\0') {
            text_to_add = NULL;
        }
    } else {
        kstrcpy(filename, args);
        text_to_add = NULL;
    }

    if (file->type == RAMDISK_INODE_TYPE_DIR) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Cannot add text to a directory.\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    char new_content[RAMDISK_DATA_SIZE_BYTES + 1];
    size_t content_length = 0;

    int bytes_read = ramdisk_readfile(file, 0, RAMDISK_DATA_SIZE_BYTES, new_content);
    if (bytes_read > 0) {
        content_length = bytes_read;
        new_content[content_length] = '\0';

        if (content_length > 0 && new_content[content_length - 1] != '\n') {
            if (content_length < RAMDISK_DATA_SIZE_BYTES) {
                new_content[content_length++] = '\n';
                new_content[content_length] = '\0';
            }
        }
    }

    size_t text_len = kstrlen(text_to_add);
    if (content_length + text_len >= RAMDISK_DATA_SIZE_BYTES) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Error: Out of Ramdisk space.");
        print_uint(RAMDISK_DATA_SIZE_BYTES);
        print(" bytes).\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    kstrcpy(new_content + content_length, text_to_add);
    content_length += text_len;
}
