BITS 16
ORG 0x7C00

%define SEL_CODE 0x08
%define SEL_DATA 0x10

start:
  cli
  xor ax, ax
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 0x7C00

  ; clear screen
  mov ax, 0x0600
  mov bh, 0x07
  mov cx, 0x0000
  mov dx, 0x184F
  int 0x10

  ; cursor top-left
  mov ah, 0x02
  mov bh, 0x00
  mov dh, 0x00
  mov dl, 0x00
  int 0x10

  ; print boot messages
  mov si, banner1
.print1:
  lodsb
  test al, al
  jz .after1
  mov ah, 0x0E
  mov bh, 0x00
  mov bl, 0x07
  int 0x10
  jmp .print1
.after1:
  mov al, 0x0D
  mov ah, 0x0E
  int 0x10
  mov al, 0x0A
  int 0x10

  mov si, banner2
.print2:
  lodsb
  test al, al
  jz .after2
  mov ah, 0x0E
  mov bh, 0x00
  mov bl, 0x07
  int 0x10
  jmp .print2
.after2:
  mov al, 0x0D
  mov ah, 0x0E
  int 0x10
  mov al, 0x0A
  int 0x10

  ; print GDT setup message
  mov si, banner3
.print3:
  lodsb
  test al, al
  jz .after3
  mov ah, 0x0E
  mov bh, 0x00
  mov bl, 0x07
  int 0x10
  jmp .print3
.after3:

  ; enable A20
  in   al, 0x92
  or   al, 0x02
  out  0x92, al

  ; set flat GDT and enter protected mode
  lgdt [gdtinfo]
  mov eax, cr0
  or  eax, 1
  mov cr0, eax
  jmp dword SEL_CODE:pmode

BITS 32
pmode:
  ; set data segment
  mov ax, SEL_DATA
  mov ds, ax

  ; print "[boot] Entering real mode..." directly to VGA text memory
  mov edi, 0xB8000
  mov esi, banner4
  mov ah, 0x07

.print4:
  lodsb
  test al, al
  jz .after4
  mov [edi], al
  mov [edi+1], ah
  add edi, 2
  jmp .print4
.after4:

  ; leave protected mode safely
  mov eax, cr0
  and eax, ~1
  mov cr0, eax
  jmp 0x0000:unreal

BITS 16
unreal:
  sti
.hang:
  hlt
  jmp .hang

banner1 db '[boot] Starting cheeseDOS...',0
banner2 db '[boot] Entering protected mode...',0
banner3 db '[boot] Setting flat GDT...',0
banner4 db '[boot] Entering real mode...',0

; ----------------------------
; GDT setup (exactly as you gave)
; ----------------------------
gdtinfo:
   dw gdt_end - gdt - 1   ; last byte in table
   dd gdt                 ; start of table

gdt:        dd 0,0        ; entry 0 is always unused
codedesc:   db 0xff, 0xff, 0, 0, 0, 10011010b, 00000000b, 0
flatdesc:   db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
gdt_end:

; ----------------------------
; Boot signature
; ----------------------------
times 510-($-$$) db 0
dw 0xAA55
