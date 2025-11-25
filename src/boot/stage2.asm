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
   mov sp, 0x9c00
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

[BITS 32]
pmode32:
   mov ax, 0x10           
   mov ds, ax
   mov es, ax
   mov fs, ax
   mov gs, ax
   mov ss, ax
   mov esp, 0x9c00        
   jmp $                   

default_isr:
   iretd

align 8
idt:
   times (256 * 8) db 0
