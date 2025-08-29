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
#include "beep.h"
#include "string.h"

static void xmas_music() {
    print("Playing: We Wish You a Merry Christmas...");
    beep(523,386);beep(698,386);beep(698,193);beep(784,193);beep(698,193);beep(659,193);
    beep(587,386);beep(587,386);beep(587,386);
    beep(784,386);beep(784,193);beep(880,193);beep(784,193);beep(698,193);
    beep(659,386);beep(523,386);beep(523,386);
    beep(880,386);beep(880,193);beep(932,193);beep(880,193);beep(784,193);
    beep(698,386);beep(587,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(523,386);
    beep(698,386);beep(698,193);beep(784,193);beep(698,193);beep(659,193);
    beep(587,386);beep(587,386);beep(587,386);
    beep(784,386);beep(784,193);beep(880,193);beep(784,193);beep(698,193);
    beep(659,386);beep(523,386);beep(523,386);
    beep(880,386);beep(880,193);beep(932,193);beep(880,193);beep(784,193);
    beep(698,386);beep(587,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(523,386);
    beep(698,386);beep(698,386);beep(698,386);
    beep(659,771);beep(659,386);
    beep(698,386);beep(659,386);beep(587,386);
    beep(523,771);beep(880,386);
    beep(932,386);beep(880,386);beep(784,386);
    beep(1047,386);beep(523,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(523,386);
    beep(698,386);beep(698,193);beep(784,193);beep(698,193);beep(659,193);
    beep(587,386);beep(587,386);beep(587,386);
    beep(784,386);beep(784,193);beep(880,193);beep(784,193);beep(698,193);
    beep(659,386);beep(523,386);beep(523,386);
    beep(880,386);beep(880,193);beep(932,193);beep(880,193);beep(784,193);
    beep(698,386);beep(587,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(523,386);
    beep(698,386);beep(698,386);beep(698,386);
    beep(659,771);beep(659,386);
    beep(698,386);beep(659,386);beep(587,386);
    beep(523,771);beep(880,386);
    beep(932,386);beep(880,386);beep(784,386);
    beep(1047,386);beep(523,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(523,386);
    beep(698,386);beep(698,193);beep(784,193);beep(698,193);beep(659,193);
    beep(587,386);beep(587,386);beep(587,386);
    beep(784,386);beep(784,193);beep(880,193);beep(784,193);beep(698,193);
    beep(659,386);beep(523,386);beep(523,386);
    beep(880,386);beep(880,193);beep(932,193);beep(880,193);beep(784,193);
    beep(698,386);beep(587,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(0,386);
    print(" OK!\n");
}

static void happy_birthday() {
    print("Playing: Happy Birthday...");

    beep(262,385);  beep(262,193);  beep(294,578);
    beep(262,578);  beep(349,578);  beep(330,1156);

    beep(262,385);  beep(262,193);  beep(294,578);
    beep(262,578);  beep(392,578);  beep(349,1156);

    beep(262,385);  beep(262,193);  beep(523,578);
    beep(440,578);  beep(349,578);  beep(330,578);

    beep(294,578);  beep(466,385);  beep(466,193);
    beep(440,578);  beep(349,578);  beep(392,578);

    beep(349,1156);

    print(" OK!\n");
}

void mus(const char* args) {
    if (!args || !*args) {
        print("Usage: mus <song>\n");
        print(" Songs:\n");
        print("  birthday\n");
        print("  xmas\n");
        return;
    }

    if (kstrcmp(args, "xmas") == 0) {
        xmas_music();
    } else if (kstrcmp(args, "birthday") == 0) {
        happy_birthday();
    } else {
        print("Unknown song. Try: xmas or birthday");
    }
}