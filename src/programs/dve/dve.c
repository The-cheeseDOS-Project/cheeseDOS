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
#include "ide.h"
#include "string.h"
#include "stdint.h"

static void size() {
    uint32_t sectors = (uint32_t)identify_buffer[60] | ((uint32_t)identify_buffer[61] << 16);
    uint64_t bytes = (uint64_t)sectors * 512;

    char buffer[32];

    if (bytes < 1024) {
        itoa((uint32_t)bytes, buffer, 10);
        print("Size:      ");
        print(buffer);
        print("B");
    } else if (bytes < (1024 * 1024)) {
        itoa((uint32_t)(bytes / 1024), buffer, 10);
        print("Size:      ");
        print(buffer);
        print("KB");
    } else if (bytes < (1024ULL * 1024 * 1024)) {
        itoa((uint32_t)(bytes / (1024 * 1024)), buffer, 10);
        print("Size:      ");
        print(buffer);
        print("MB");
    } else {
        itoa((uint32_t)(bytes / (1024ULL * 1024 * 1024)), buffer, 10);
        print("Size:      ");
        print(buffer);
        print("GB");
    }
}

//static void serial() {
//    char serial[21];
//    for (int i = 0; i < 10; i++) {
//        serial[2*i]   = identify_buffer[10 + i] >> 8;
//        serial[2*i+1] = identify_buffer[10 + i] & 0xFF;
//    }
//    serial[20] = '\0';
//    print("Serial:    ");
//    print(serial);
//}

static void model() {
    char model[41];
    for (int i = 0; i < 20; i++) {
        model[2*i]   = identify_buffer[27 + i] >> 8;
        model[2*i+1] = identify_buffer[27 + i] & 0xFF;
    }
    model[40] = '\0';
    print("Model:     ");
    print(model);
}

//static void firmware() {
//    char firmware[9];
//    for (int i = 0; i < 4; i++) {
//        firmware[2*i]   = identify_buffer[23 + i] >> 8;
//        firmware[2*i+1] = identify_buffer[23 + i] & 0xFF;
//    }
//    firmware[8] = '\0';
//    print("Firmware:  ");
//    print(firmware);
//}

static void cylinder() {
    uint16_t cylinders = identify_buffer[1];
    char buffer[10];
    itoa(cylinders, buffer, 10);
    print("Cylinders: ");
    print(buffer);
}

static void heads() {
    uint16_t heads = identify_buffer[3];
    char buffer[10];
    itoa(heads, buffer, 10);
    print("Heads:     ");
    print(buffer);
}

static void sectors() {
    uint16_t sectors = identify_buffer[6];
    char buffer[10];
    itoa(sectors, buffer, 10);
    print("Sectors:   ");
    print(buffer);
}

static void cache() {
    uint16_t blocks = identify_buffer[5];
    uint32_t bytes = blocks * 512;

    char buffer[16];

    if (bytes == 0) {
        print("Cache:     N/A\n");
        return;
    }

    if (bytes < 1024) {
        itoa(bytes, buffer, 10);
        print("Cache:     ");
        print(buffer);
        print(" bytes\n");
    } else if (bytes < (1024 * 1024)) {
        itoa(bytes / 1024, buffer, 10);
        print("Cache:     ");
        print(buffer);
        print("KB\n");
    } else {
        itoa(bytes / (1024 * 1024), buffer, 10);
        print("Cache:     ");
        print(buffer);
        print("MB\n");
    }
}

void dve(const char *unused) {
    (void)unused;
    if (identify_buffer[0] == 0) {
        print("No IDE Master detected\n");
        return;
    }

    size();
    print("\n");
//    serial();
//    print("\n");
    model();
    print("\n");
//    firmware();
//    print("\n");
    cylinder();
    print("\n");
    heads();
    print("\n");
    sectors();
    print("\n");
    cache();
}
