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
#include "vga.h"
#include "beep.h"
#include "timer.h"

void free_software_song() {
    print("Join us now and share the software;\n");
    beep(587, 500); delay(50);   
    beep(523, 250); delay(50);   
    beep(494, 500); delay(50);   
    beep(440, 500); delay(50);   
    beep(494, 500); delay(50);   
    beep(523, 250); delay(50);   
    beep(494, 250); delay(50);   
    beep(440, 250); delay(50);   
    beep(392, 500); delay(50);   
   
    print("You'll be free, hackers, you'll be free.\n");
    beep(392, 750); delay(50);   
    beep(440, 750); delay(50);   
    beep(494, 250); delay(50);   
    beep(523, 750); delay(50);   
    beep(494, 500); delay(50);   
    beep(494, 250); delay(50);   
    beep(587, 250); delay(50);
    beep(440, 750); delay(50);   
    beep(440, 1000); delay(50);  
    beep(587, 500); delay(50);
    beep(523, 250); delay(50);   
    beep(494, 1000); delay(50);  
    
    print("Hoarders can get piles of money,\n");
    beep(587, 500); delay(50);   
    beep(523, 250); delay(50);   
    beep(494, 500); delay(50);   
    beep(440, 500); delay(50);   
    beep(494, 500); delay(50);   
    beep(523, 250); delay(50);   
    beep(494, 250); delay(50);   
    beep(440, 250); delay(50);   
    beep(392, 500); delay(50);   
    
    print("That is true, hackers, that is true.\n");
    beep(392, 750); delay(50);   
    beep(440, 750); delay(50);   
    beep(494, 250); delay(50);   
    beep(523, 750); delay(50);   
    beep(494, 500); delay(50);   
    beep(494, 250); delay(50);   
    beep(587, 250); delay(50);   
    beep(440, 750); delay(50);   
    beep(440, 1000); delay(50);  
    beep(440, 1750); delay(100); 
}
