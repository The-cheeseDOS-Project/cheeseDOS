# cheeseDOS - My x86 DOS
# Copyright (C) 2025  Connor Thomson
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

.intel_syntax  noprefix  
.section .text
.align 4
.long 0x1BADB002
.long 0
.long -(0x1BADB002)

.global stack_top

.global _start
_start:
    call kernel_main
    cli
    hlt
