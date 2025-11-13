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

#include "string.h"  
#include "vga.h"     
#include "serial.h"  
#include "shell.h"   
#include "ramdisk.h" 
#include "ide.h"     
#include "log.h"     

void init() {
    char buf[16]; 

    sprint("Switching to graphics mode... ");
    graphics_mode();
    sprint("Done!\n");

    gprint("Starting cheeseDOS...\n");

    bprint("Loading ");
    itoa(RAMDISK_DATA_SIZE_BYTES, buf, 10);
    bprint(buf);
    bprint("B RAM Disk... ");

    ramdisk_init(); 

    success("Done!\n");

    bprint("Looking for IDE master... "); 

    if (ide_detect()) {
        success("Found!\n");

        bprint("Loading IDE Drive... ");

        if (ide_init()) {
            success("Done!\n");
        } else { 
            error("Failed!\n");
        }
    } else { 
        error("Not Found!\n");
    }

    shell_run();
}
