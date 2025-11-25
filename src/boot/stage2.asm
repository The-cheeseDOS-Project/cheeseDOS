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
   mov [BootDrive], dl
   xor ax, ax       
   mov ds, ax             
   mov ss, ax             
   mov sp, 0x9c00
   mov si, enable_a20
   call print
   call a20
   mov si, set_idt
   call print
   call setup_idt
   mov si, set_gdt
   call print
   lgdt [gdtinfo]         
   mov si, enter_pmode
   call print
   mov eax, cr0          
   or al, 1
   mov cr0, eax
   mov bx, 0x10
   mov ds, bx
   mov es, bx
   mov fs, bx
   mov gs, bx
   mov ss, bx
   and al, 0xFE            
   mov cr0, eax
   xor ax, ax
   mov ds, ax
   mov ss, ax
   mov sp, 0x9c00
   sti                     
   mov si, enter_unreal
   call print
   cli                     
   jmp boot

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
    mov si, enter_pmode
    call print
    lidt [idtinfo]
    lgdt [gdtinfo]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:pmode32

setup_idt:
   push ax
   push cx
   push di
   push es
   xor ax, ax
   mov es, ax
   mov di, idt
   mov cx, 256
.loop:
   mov eax, default_isr
   mov [es:di], ax
   add di, 2
   mov word [es:di], 0x08
   add di, 2
   mov byte [es:di], 0
   inc di
   mov byte [es:di], 0x8E
   inc di
   shr eax, 16
   mov [es:di], ax
   add di, 2
   loop .loop
   pop es
   pop di
   pop cx
   pop ax
   ret

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

a20:
    in al, 0x64
    test al, 2
    jnz a20
    mov al, 0xD1
    out 0x64, al
.wait1:
    in al, 0x64
    test al, 2
    jnz .wait1
    mov al, 0xDF
    out 0x60, al
.wait2:
    in al, 0x64
    test al, 2
    jnz .wait2
    ret

gdtinfo:
   dw gdt_end - gdt - 1   
   dd gdt                 

gdt:	
   dd 0,0                 

codedesc:                 
   dw 0xFFFF              
   dw 0x0000              
   db 0x00                
   db 10011010b           
   db 11001111b           
   db 0x00                

flatdesc:                 
   dw 0xFFFF              
   dw 0x0000              
   db 0x00                
   db 10010010b           
   db 11001111b           
   db 0x00                

gdt_end:

idtinfo:
   dw (256 * 8) - 1       
   dd idt                 

set_idt db "Setting IDT...", 0x0D, 0x0A, 0
enter_pmode db "Entering protected mode...", 0x0D, 0x0A, 0
set_gdt db "Setting GDT...", 0x0D, 0x0A, 0
enter_unreal db "Entering unreal mode...", 0x0D, 0x0A, 0
load_disk db "Loading kernel...",0x0D, 0x0A, 0
no_kernel db "No kernel found!", 0x0D, 0x0A, 0
disk_error db "Disk read error!", 0x0D, 0x0A, 0
enable_a20 db "Enabling A20...", 0x0D, 0x0A, 0
BootDrive db 0

[BITS 32]
pmode32:
    mov ax, 0x10           
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9c00    
    mov edi, 0xB8000 + (8 * 160)
    mov esi, start_kernel
    call print32
    jmp 0x08:0x10000
    mov esi, kernel_exit
    call print32
    cli    
.loop:
    hlt
    jmp .loop

print32:
    mov ah, 0x07
    lodsb
    test al, al
    jz .done
    stosw
    jmp print32
.done:
    ret

default_isr:
    iretd

start_kernel db "Starting kernel...", 0x0D, 0x0A, 0
kernel_exit db "Kernel has exited!", 0

align 8
idt:
   times (256 * 8) db 0
