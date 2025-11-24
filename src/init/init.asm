[org 0x0000]             ; Tell assembler this code's offset will be 0x0000
                         ; (The physical load address is 0x10000, set by the bootloader)
start:
    cli
    mov ax, 0x1000       ; Load the correct segment base
    mov ds, ax           ; Set data segment to 0x1000
    mov si, message      ; SI is the offset (0x000C after the ORG 0x0000 base)

print_loop:
    lodsb
    or al, al
    jz done
    mov ah, 0x0E
    int 0x10
    jmp print_loop

done:
    hlt

message db 'Hello, world!', 0
