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

   mov si, load_gdt
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
   hlt

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
   db 0xff,0xff,0,0,0,10011010b,11001111b,0

flatdesc:
   db 0xff,0xff,0,0,0,10010010b,11001111b,0

gdt_end:

enter_pmode db "Entering protected mode...", 0x0D, 0x0A, 0
load_gdt db "Loading GDT...", 0x0D, 0x0A, 0
enter_unreal db "Entering unreal mode...", 0x0D, 0x0A, 0

