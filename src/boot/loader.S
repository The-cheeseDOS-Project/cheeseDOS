/*
 * cheeseLDR - single sector x86 ELF loader for cheeseDOS
 * Copyright (C) 2025 Roman Kapl
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

.intel_syntax noprefix
.code16

.include "elf.S"
.include "gdt.S"

.equ STACK_TOP, 0x7C00
.equ SELF_LOAD, 0x7C00
.equ ELF_HDR_LOAD, 0x7E00

.equ SECT_SIZE, 512
.equ SECT_SHIFT, 9
.equ GEOM_SECTORS, 18
.equ GEOM_CYLINDERS, 80
.equ GEOM_HEADS, 2

.section .text
.global start
start:

cli

#### Setup 

# make sure cs = 0
jmp 0:start2
start2:

# setup stack
xor cx, cx
mov ss, cx
mov sp, STACK_TOP

mov ds, cx

mov ax, 0xB800
mov es, ax
xor di, di
mov ah, 0x0F
mov al, ' '
mov cx, 2000

.clear_loop:
stosw
loop .clear_loop

mov dx, 0x03D4
mov al, 0x0F
out dx, al
mov dx, 0x03D5
mov al, 0
out dx, al

mov dx, 0x03D4
mov al, 0x0E
out dx, al
mov dx, 0x03D5
mov al, 0
out dx, al

mov si, offset msg_banner

mov ah, 0x02
mov bh, 0x00
mov dh, 0x00
mov dl, 0x00
int 0x10

call print

# setup ds to point to ELF_HDR_LOAD
mov ax, ELF_HDR_LOAD/16
mov ds, ax

# read second sector into ELF_HDR_LOAD (current ds:0)
mov ax, 0x0201
mov cx, 0x0002
xor dx, dx
mov bx, ds
mov es, bx
xor bx, bx
int 0x13
jc err

#### Load Kernel 

# si = current phdr pointer
mov si, [elf_phoff]
# cl = remaining segments
mov cl, [elf_phnum]

load_segment:
push cx

mov eax, [si + elf_seg_type]
dec eax
jnz skip_seg

# es = load addr / 16
mov ebx, [si + elf_seg_paddr]
shr ebx, 4
mov es, bx

# eax = (file_offset + SECT)/SECT
mov eax, [si + elf_seg_file_offset]
shr eax, SECT_SHIFT
inc ax

# save drive number we got from bios
push dx

# decode into CHS
xor dx, dx
mov bx, GEOM_SECTORS
div bx
mov cl, dl
inc cl # cl = sectors = remainder + 1
mov bl, GEOM_HEADS
div bl
pop dx
mov dh, ah # dl = heads = remainder
mov ch, al # ch = cylinders = quotient

# al = filesize / SECT with rounding up
xor ebx, ebx
mov bx, SECT_SIZE - 1
mov eax, [si + elf_seg_filesz]
add eax, ebx
shr eax, SECT_SHIFT

# function code, drive, no offset
mov ah, 0x02
xor bx, bx

int 0x13
jc err

# clear the bss part, we assume max 64k segment here
mov ecx, [si + elf_seg_memsz]
mov edi, [si + elf_seg_filesz]
sub ecx, edi
xor al, al
cld
rep stosb

# next segment
skip_seg:
pop cx
add si, elf_seg_struct_size
loop short load_segment

mov esi, [elf_entry]

#### Jump to kernel 

# construct GDT pointer + far pointer + GDT in the space previously occupied by elf headers
.equ gdtp, 0
.equ gdt, 8
cld
# stosb dest
mov ax, ds
mov es, ax
xor di, di

# GDT pointer
xor eax, eax
mov al, 3*8 - 1
stosw
mov ax, ELF_HDR_LOAD + gdt
stosd
stosw # padd

# null descriptor
xor eax, eax
stosd
stosd
# two descriptors from template
lea bx, [gdt_code]
mov eax, cs:[bx]
mov edx, cs:[bx+4]
stosd
xchg eax, edx
stosd
xchg eax, edx
stosd
xchg eax, edx
stosd
mov al, gdt_a_present | gdt_a_nosys | gdt_a_dpl0 | gdt_a_rw | gdt_a_accessed
mov ds:[gdt + 8*2 + 5], al

# fast enable A20 and hope for the best
in al, 0x92
or al, 2
out 0x92, al

# enter protected mode
lgdt [gdtp]
mov eax, cr0
or eax, 1
mov cr0, eax

# jump to kernel
jmp 8:prot32

.code32
prot32:
xor eax, eax
mov al, 16
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

jmp esi

.code16
#### Library code

# ds:si = null terminated message
print:
cld
mov ah, 0x0E
xor bx, bx

.loop:
lodsb
test al, al
jz .ret
int 0x10
jmp .loop

.ret:
ret

err:
xor bx, bx
mov ds, bx
mov si, offset msg_error
call print
hlt:
jmp hlt

#### Data

msg_banner:
.ascii "Loading kernel...\r\n\0"
msg_error:
.ascii "Failed! Please reboot and try again...\0"

gdt_code:
    .word 0xFFFF # limit
    .word 0x0000  # base
    .byte 0x0     # base
    .byte gdt_a_present | gdt_a_nosys | gdt_a_dpl0 | gdt_a_exec | gdt_a_rw | gdt_a_accessed
    .byte gdt_fl_32b | gdt_fl_pglimit | 0xF # flags + few limit bits
    .byte 0x0      # base

.space 0x200 - 2 - (. - start)
.word 0xaa55
