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

ENTRY(start) /* Set entry point to start */

MEMORY
{
  CODE (rwx)  : ORIGIN =  0x7c00, LENGTH = 512 /* Make it BIOS friendly */
}

SECTIONS
{
  . = ORIGIN(CODE); /* Set the location to the start of the CODE region */
  .text : {         /* Contains the actual code */
    *(.text)        /* Include all .text sections */
  } > CODE          /* Place this section in the CODE memory region */

  /DISCARD/ : { /* Discard unwanted sections to save space */
    *(.note.*)  /* Remove all note sections */
  }
}
