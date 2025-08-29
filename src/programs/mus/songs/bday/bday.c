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

void bday_music() {
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