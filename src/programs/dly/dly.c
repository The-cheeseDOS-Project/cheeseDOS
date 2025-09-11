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

#include "timer.h"

void dly(const char* args) {
    uint32_t ms = 1000;

    char buf[16];
    int i = 0;

    while (args[i] && i < 15 && args[i] >= '0' && args[i] <= '9') {
        buf[i] = args[i];
        i++;
    }

    ms = 0;
    for (i = 0; buf[i]; ++i)
        ms = ms * 10 + (buf[i] - '0');

    delay(ms);
}