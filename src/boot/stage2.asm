; cheeseDOS - My x86 DOS
; Copyright (C) 2025  Connor Thomson
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.

ORG 0x7E00

[BITS 16]
stage2:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [BootDrive], dl

    mov ah, 0x02
    mov bh, 0x00
    mov dh, 0x02
    mov dl, 0x00
    int 0x10

    mov si, load_gdt
    call print
    lgdt [gdtinfo]
    mov si, enter_unreal
    call print
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pmode

print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp print
.done:
    ret

boot:
    mov si, load_disk
    call print

    mov dl, [BootDrive]
    xor ax, ax
    int 0x13
    jc disk_error

    mov ax, 0x1000
    mov es, ax
    mov bx, 0

    mov dl, [BootDrive]
    mov dh, 0
    mov ch, 0
    mov cl, 4
    mov al, 16
    mov ah, 0x02

    int 0x13
    jc disk_error

    cmp al, 0
    je disk_error

    mov ax, [es:0]
    cmp ax, 0
    je no_kernel

    jmp kernel

disk_error:
    mov si, error_disk_read
    call print
    jmp halt

no_kernel:
    mov si, error_no_kernel
    call print
    jmp halt

kernel:
    mov si, enter_pmode
    call print

    lgdt [gdtinfo]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:kernel32

halt:
    cli
    hlt
    jmp halt

load_disk db "Loading kernel...",0x0D, 0x0A, 0
load_gdt db "Loading GDT...", 0x0D, 0x0A, 0
enter_pmode db "Entering protected mode...", 0x0D, 0x0A, 0
enter_unreal db "Entering unREAL mode...", 0x0D, 0x0A, 0
error_disk_read db "Disk read error!", 0x0D, 0x0A, 0
error_no_kernel db "No kernel found!", 0x0D, 0x0A, 0
BootDrive db 0

align 8
gdt:
    dd 0,0
flatcode:
    db 0xff,0xff,0,0,0,10011010b,11001111b,0
flatdata:
    db 0xff,0xff,0,0,0,10010010b,11001111b,0
gdt_end:
gdtinfo:
    dw gdt_end - gdt - 1
    dd gdt

[BITS 32]
pmode:
    os16 mov bx, 0x10
    mov ebx, 0x10
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    mov eax, cr0 
    and eax, 0xFFFFFFFE
    mov cr0, eax
    jmp 0x0:boot

kernel32:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    mov esi, start_kernel
    mov edi, 0xB8000
    mov ah, 0x0F  
.print_loop:
    lodsb
    or al, al
    jz .done
    stosw
    jmp .print_loop
.done:

.hang:
    cli
    hlt
    jmp .hang

start_kernel db "Starting kernel...", 0
