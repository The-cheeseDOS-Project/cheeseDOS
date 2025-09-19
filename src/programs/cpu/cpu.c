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

// NOTE: This needs CPUID and this is super quick and dirty.

#include "vga.h"

char vendor[13];
int family = 0;
int model = 0;
int stepping = 0;

void cpu(char const* *unused) {
    (void)unused;

    __asm__ volatile (
        "mov $0, %%eax\n\t"
        "cpuid\n\t"
        "mov %%ebx, %0\n\t"
        "mov %%edx, %1\n\t"
        "mov %%ecx, %2\n\t"
        : "=m"(vendor[0]), "=m"(vendor[4]), "=m"(vendor[8])
        :
        : "eax", "ebx", "ecx", "edx"
        );
        vendor[12] = 0;

        __asm__ volatile (
        "mov $1, %%eax\n\t"
        "cpuid\n\t"
        "mov %%eax, %0\n\t"
        : "=m"(family)
        :
        : "eax"
    );

    stepping = family & 0xF;
    model = (family >> 4) & 0xF;
    family = (family >> 8) & 0xF;

    char buf[2];
    buf[1] = '\0';

    print("Vendor   : ");
    print(vendor);
    print("\nFamily   : ");
    if (family >= 10) {
        buf[0] = '1';
        print(buf);
    }
    buf[0] = '0' + (family % 10);
    print(buf);

    print("\nModel    : ");
    if (model >= 10) {
        buf[0] = '1';
        print(buf);
    }
    buf[0] = '0' + (model % 10);
    print(buf);

    print("\nStepping : ");
    if (stepping >= 10) {
        buf[0] = '1';
        print(buf);
    }
    buf[0] = '0' + (stepping % 10);
    print(buf);

    print("\n");
}
