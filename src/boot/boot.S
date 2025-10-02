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

.intel_syntax noprefix  # Use intel syntax
.code16  # Use 16 bit real mode

.equ STACK_TOP, 0x7C00  # Top of stack
.equ SELF_LOAD, 0x7C00  # Location of bootloader
.equ ELF_HDR_LOAD, 0x7E00  # Where cheeseDOS will be loaded

.equ SECT_SIZE, 512  # Sector size
.equ SECT_SHIFT, 9  # Number of bits needed to do a bit shift 
.equ GEOM_SECTORS, 18  # Sectors
.equ GEOM_CYLINDERS, 80  # Cylinders
.equ GEOM_HEADS, 2  # Heads

.equ elf_phoff, 0x1C  # Offset from beginning of the ELF file header to the start of the program header table
.equ elf_phnum, 0x2C  # Number of entries in PHT
.equ elf_entry, 0x18  # Virtual memory address of entry point

.equ elf_seg_type, 0x00  # Segment type field offset
.equ elf_seg_file_offset, 0x04  # Segment start offset
.equ elf_seg_paddr, 0x0C  # Address of the segment
.equ elf_seg_filesz, 0x10  # Size of segment
.equ elf_seg_memsz, 0x14  # Size of segment in memory
.equ elf_seg_struct_size, 0x20  # Size of a program header entry

.equ gdt_fl_pglimit, 0x80  # GDT page granularity
.equ gdt_fl_32b, 0x40  # GDT 32-bit segment

.equ gdt_a_present, 0x80  # GDT present
.equ gdt_a_dpl0, 0x00  # GDT ring 0
.equ gdt_a_nosys, 0x10  # GDT non system segment
.equ gdt_a_exec, 0x08  # GDT executable
.equ gdt_a_conforming, 0x04  # GDT conforming code
.equ gdt_a_grow_down, 0x04  # GDT grow down stack
.equ gdt_a_rw, 0x2  # GDT readable/writable
.equ gdt_a_accessed, 0x1  # GDT accessed bit

.section .text  # Start of code section
.global start  # Export entry point
start:  # Entry point

cli  # Disable interrupts
jmp 0:start2  # Far jump to flush prefetch queue
start2:  # The "real" stuff begins here

xor cx, cx  # Zero CX
mov ss, cx  # Set stack segment to 0
mov sp, STACK_TOP  # Set stack pointer to begining of the bootloader
mov ds, cx  # Set data segment to 0

mov ax, 0xB800  # Video memory segment
mov es, ax  # Set ES to video memory
xor di, di  # Start at offset 0
mov ah, 0x0F  # Change text color to white on black

# Clear screen
mov al, ' '
mov cx, 2000
.clear_loop:
stosw
loop .clear_loop

mov ah, 0x02
xor bh, bh
xor dh, dh
xor dl, dl
int 0x10

mov si, offset msg_loading
call print

mov si, drives

scan_floppy:
    lodsb
    test al, al
    jz err
    mov dl, al

    mov ax, 0x0201
    mov cx, 0x0002
    xor dx, dx
    mov bx, ELF_HDR_LOAD/16
    mov es, bx
    xor bx, bx
    int 0x13
    jc scan_floppy

    jmp load_init

load_init:
    mov ax, ELF_HDR_LOAD/16
    mov ds, ax
    mov si, [elf_phoff]
    mov cl, [elf_phnum]

load_segment:
    push cx
    mov eax, [si + elf_seg_type]
    dec eax
    jnz skip_seg

    mov ebx, [si + elf_seg_paddr]
    shr ebx, 4
    mov es, bx

    mov eax, [si + elf_seg_file_offset]
    shr eax, SECT_SHIFT
    inc ax

    push dx
    xor dx, dx
    mov bx, GEOM_SECTORS
    div bx
    mov cl, dl
    inc cl
    mov bl, GEOM_HEADS
    div bl
    pop dx
    mov dh, ah
    mov ch, al

    xor ebx, ebx
    mov bx, SECT_SIZE - 1
    mov eax, [si + elf_seg_filesz]
    add eax, ebx
    shr eax, SECT_SHIFT

    mov ah, 0x02
    xor bx, bx
    int 0x13
    jc err

skip_seg:
    pop cx
    add si, elf_seg_struct_size
    loop short load_segment

    mov esi, [elf_entry]

    .equ gdtp, 0
    .equ gdt, 8
    cld
    mov ax, ds
    mov es, ax
    xor di, di

    xor eax, eax
    mov al, 3*8 - 1
    stosw
    mov ax, ELF_HDR_LOAD + gdt
    stosd
    stosw

    xor eax, eax
    stosd
    stosd
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

    in al, 0x92
    or al, 2
    out 0x92, al

    lgdt [gdtp]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

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
    mov ds, bx
    mov si, offset msg_failed
    call print
    jmp halt_loop

drives:
    .byte 0x00
    .byte 0x01
    .byte 0x00

msg_loading:
    .ascii "Starting cheeseDOS...\r\0"
msg_failed:
    .ascii "Starting cheeseDOS... Failed!\0"

gdt_code:
    .word 0xFFFF
    .word 0x0000
    .byte 0x0
    .byte gdt_a_present | gdt_a_nosys | gdt_a_dpl0 | gdt_a_exec | gdt_a_rw | gdt_a_accessed
    .byte gdt_fl_32b | gdt_fl_pglimit | 0xF
    .byte 0x0

halt_loop:
    hlt
    jmp halt_loop

.space 0x200 - 2 - (. - start)
.word 0xaa55
