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
#include "string.h"
#include "shell.h"

void rep(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: rep <times> <command> [args...]\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    char times_buf[16];
    const char *p = args;
    size_t i = 0;
    while (*p && *p != ' ' && i + 1 < sizeof(times_buf)) {
        times_buf[i++] = *p++;
    }
    times_buf[i] = '\0';

    if (i == 0) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: rep <times> <command> [args...]\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    int times = 0;
    for (size_t j = 0; j < kstrlen(times_buf); ++j) {
        char c = times_buf[j];
        if (c < '0' || c > '9') {
            set_text_color(COLOR_RED, COLOR_BLACK);
            print("Error: times must be a positive integer\n");
            set_text_color(default_text_fg_color, default_text_bg_color);
            return;
        }
        times = times * 10 + (c - '0');
        if (times < 0) break;
    }

    if (times <= 0) return;

    while (*p == ' ') p++;
    if (*p == '\0') {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: rep <times> <command>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    char cmd_buf[256];
    size_t cmd_len = kstrlen(p);
    if (cmd_len >= sizeof(cmd_buf)) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Error: command too long\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    kstrcpy(cmd_buf, p);

    for (int t = 0; t < times; ++t) {
        shell_execute(cmd_buf);
    }
}
