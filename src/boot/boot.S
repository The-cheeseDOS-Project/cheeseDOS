.intel_syntax noprefix
.code16
.global start

start:
    cli
    xor ax,ax
    mov ss,ax
    mov sp,0x7C00

    mov ax,0x07C0
    mov ds,ax
    mov es,ax

    sti

    mov ah,0x00
    mov al,0x03
    int 0x10

    mov si, msg
print_string:
    lodsb
    test al,al
    jz hang
    mov ah,0x0E
    int 0x10
    jmp print_string

hang:
    jmp hang

msg:
    .asciz "Starting cheeseDOS...\r\n"

.org 510
.word 0xAA55
