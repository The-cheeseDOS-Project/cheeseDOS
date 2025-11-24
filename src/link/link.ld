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

ENTRY(kmain)
OUTPUT_FORMAT(binary)

SECTIONS {
    . = 0x10000;
    
    .text : ALIGN(16) {
        *(.text)
        *(.text)
        *(.text.*)
    }
    
    .rodata : ALIGN(16) {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data : ALIGN(16) {
        *(.data)
        *(.data.*)
    }
    
    .bss : ALIGN(16) {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }
    
    /DISCARD/ : {
        *(.interp)
        *(.dynsym)
        *(.dynstr)
        *(.gnu.hash)
        *(.hash)
        *(.note.gnu.property)
        *(.note.gnu.build-id)
        *(.note.*)
        *(.rel.dyn)
        *(.rela.dyn)
        *(.rel.*)
        *(.rela.*)
        *(.dynamic)
        *(.got)
        *(.got.plt)
        *(.plt)
        *(.debug_*)
        *(.comment)
        *(.eh_frame)
        *(.eh_frame_hdr)
        *(.gnu.warning)
    }
}
