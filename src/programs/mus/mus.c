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
#include "string.h"
#include "free_software_song.h"

void mus(const char* args) {
    if (!args || !*args) {
        print("Usage: mus <song>\n");
        print(" Songs:\n");
        print("  free_software_song\n");
        return;
    }

    if (kstrcmp(args, "free_software_song") == 0) {
        free_software_song();
    } else {
        print("Unknown song, Run mus for help.\n");
    }
}
