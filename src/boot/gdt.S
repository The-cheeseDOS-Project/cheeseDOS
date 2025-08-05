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

.equ gdt_fl_pglimit, 0x80
.equ gdt_fl_32b, 0x40

.equ gdt_a_present, 0x80
.equ gdt_a_dpl0, 0x00
.equ gdt_a_nosys, 0x10
.equ gdt_a_exec, 0x08
.equ gdt_a_conforming, 0x04
.equ gdt_a_grow_down, 0x04
.equ gdt_a_rw, 0x2
.equ gdt_a_accessed, 0x1
