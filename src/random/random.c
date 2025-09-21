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

#include "stdint.h"

static uint32_t seed = 123456789;
static uint16_t rand_result = 0;

void random() {
    seed = seed * 1103515245 + 12345;
    rand_result = (seed >> 16) & 0x7FFF;
}

unsigned short random_get(void) {
    return rand_result;
}

void random_seed(unsigned int s) {
    seed = s;
}
