[org 0x7C00]
[bits 16]

start:
    push cs
    pop ds
    mov ax, 0x0003
    int 0x10

    mov ah, 0x01
    mov cx, 0x2000
    int 0x10

    mov si, loading
    call print
    
    mov ax, 0x0000
    mov es, ax
    mov bx, 0x7E00
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [BootDrive]
    int 0x13
    jc disk_error
    
    mov si, success
    call print
    jmp 0x0000:0x7E00

disk_error:
    mov si, error
    call print
    call print_hex
    call newline

    jmp $

print:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp .loop
.done:
    popa
    ret

print_hex:
    pusha
    mov bl, al
    shr al, 4
    call .nibble
    mov al, bl
    and al, 0x0F
    call .nibble
    popa
    ret
.nibble:
    cmp al, 9
    jbe .digit
    add al, 7
.digit:
    add al, 0x30
    mov ah, 0x0E
    int 0x10
    ret

newline:
    pusha
    mov al, 0x0D
    mov ah, 0x0E
    int 0x10
    mov al, 0x0A
    mov ah, 0x0E
    int 0x10
    popa
    ret

loading db "Loading stage 2...", 0x0D, 0x0A, 0
success db "Starting stage 2...", 0x0D, 0x0A, 0
error db "Disk error: 0x", 0

BootDrive db 0

times 510-($-$$) db 0
dw 0xAA55
