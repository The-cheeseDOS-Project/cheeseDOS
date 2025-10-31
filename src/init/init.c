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

#include "string.h"  // For "itoa()"
#include "vga.h"     // For "print()" and "set_text_color()"
#include "serial.h"  // For "bprint()" and "sprint()"
#include "shell.h"   // For "shell_run()"
#include "ramdisk.h" // For "ramdisk_init()" and "RAMDISK_DATA_SIZE_BYTES"
#include "ide.h"     // For "ide_detect()" and "ide_init()"
#include "log.h"     // For "error()"

void init() {
    char buf[16]; // Buffer for converting numbers to strings

    bprint("Starting cheeseDOS...\n"); // Retain bootloader message and print it out on serial

    // Print out "Loading <SIZE>B RAM Disk..." on both screen and serial
    bprint("Loading ");
    itoa(RAMDISK_DATA_SIZE_BYTES, buf, 10);
    bprint(buf);
    bprint("B RAM Disk...");

    ramdisk_init(); // Start the RAM Disk

    // Success " Done!\n"
    success(" Done!\n");
    
    bprint("Looking for IDE master..."); // Print out "Looking for IDE master... " on both screen and serial
    // Check if the IDE driver finds a drive, if so run this loop
    if (ide_detect()) {
        // Success " Found!\n"
        success(" Found!\n");

        // Print "Loading IDE Drive..." on screen and serial
        bprint("Loading IDE Drive...");

        // Check if the IDE drive has been loaded successfully, if so then run this loop
        if (ide_init()) {
            // Success " Done!\n"
            success(" Done!\n");
        } else { // If IDE driver failed, run this loop
            // Error " Failed!\n"
            error(" Failed!\n");
        }
    } else { // If IDE driver can not find drive, run this loop
        // Error " Not Found!\n"
        error("Not Found!\n");
    }

    // Run shell
    shell_run();
}
