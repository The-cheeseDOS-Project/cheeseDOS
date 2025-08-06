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
#include "io.h"

#define PIT_CMD  0x43
#define PIT_CH0  0x40
#define PIT_FREQ 1193182

static uint32_t elapsed_ms = 0;

void timer_init(void) {
    outb(PIT_CMD, 0x34);
    uint16_t reload = PIT_FREQ / 1000;
    outb(PIT_CH0, reload & 0xFF);
    outb(PIT_CH0, reload >> 8);
}

uint32_t timer_millis(void) {
    static uint16_t prev_count = 0;
    static uint32_t tick_accum = 0;

    outb(PIT_CMD, 0x00);
    uint8_t lo = inb(PIT_CH0);
    uint8_t hi = inb(PIT_CH0);
    uint16_t count = (hi << 8) | lo;

    uint16_t reload = PIT_FREQ / 1000;
    uint16_t delta;

    if (prev_count >= count) {
        delta = prev_count - count;
    } else {
        delta = reload - count + prev_count;
    }

    prev_count = count;
    tick_accum += delta;

    while (tick_accum >= reload) {
        tick_accum -= reload;
        ++elapsed_ms;
    }

    return elapsed_ms;
}

void delay(uint32_t ms) {
    uint32_t start = timer_millis();
    while ((timer_millis() - start) < ms);
}
