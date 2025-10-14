/*
 * cheeseDOS - My x86 DOS
 * Copyright (C) 2025 Connor Thomson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "stdint.h"

#define RANDOMIZE 500U
#define SEED 123456789U

static uint32_t seed = SEED;
static uint16_t rand_result = 0;

static uint32_t lcg(uint32_t s) {
    return (s * 1103515245U + 12345U);
}

static uint16_t extract_rand(uint32_t s) {
    return (uint16_t)(s >> 17);
}

void random() {
    uint32_t current_seed = seed;

    for (unsigned int i = 0; i < RANDOMIZE; i++) {
        current_seed = lcg(current_seed);
    }

    rand_result = extract_rand(current_seed);
    seed = current_seed;
}

unsigned short random_get(void) {
    return rand_result;
}

void random_seed(unsigned int s) {
    if (s == 0) {
        seed = SEED;
    } else {
        seed = s;
    }
}
