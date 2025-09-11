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

static unsigned int seed = 123456789;

static unsigned int rand_bare() {
    seed = (1664525 * seed + 1013904223);
    return seed;
}

static unsigned int parse_int(const char* str) {
    unsigned int result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

static void int_to_str(unsigned int value, char* buf) {
    char temp[10];
    int i = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    for (int j = 0; j < i; j++) {
        buf[j] = temp[i - j - 1];
    }
    buf[i] = '\0';
}

void die(const char* args) {
    unsigned int max = parse_int(args);
    if (max == 0) max = 10;

    unsigned int random_number = (rand_bare() % max) + 1;

    char buf[12];
    int_to_str(random_number, buf);

    print(buf);
    print("\n");
}