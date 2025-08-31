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
#include "beep.h"

void bep(const char* args) {
    int hz = 720;
    int ms = 10;

    char num1[16];
    char num2[16];
    int i = 0, j = 0;

    while (args[i] && args[i] != ' ' && i < 15) {
        num1[i] = args[i];
        i++;
    }
    if (args[i] == ' ') i++;

    while (args[i] && j < 15) {
        num2[j++] = args[i++];
    }

    hz = 0;
    for (i = 0; num1[i] >= '0' && num1[i] <= '9'; ++i)
        hz = hz * 10 + (num1[i] - '0');

    ms = 0;
    for (i = 0; num2[i] >= '0' && num2[i] <= '9'; ++i)
        ms = ms * 10 + (num2[i] - '0');

    if (hz == 0) hz = 720;
    if (ms == 0) ms = 360;

    beep(hz, ms);
}
