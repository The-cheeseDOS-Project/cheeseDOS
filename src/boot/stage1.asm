[org 0x7C00]
[bits 16]
start:
    push cs
    pop ds
    mov ax, 0x0003
    int 0x10
    mov si, msg1
.p1:
    lodsb
    test al, al
    jz .p1d
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp .p1
.p1d:
    mov [BootDrive], dl
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
    jmp 0x0000:0x7E00
disk_error:
    mov si, errmsg
.de:
    lodsb
    test al, al
    jz .hang
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x0C
    int 0x10
    jmp .de
.hang:
    jmp $
msg1 db "Starting cheeseDOS...",0
errmsg db "Disk read error!",0
BootDrive db 0
times 510-($-$$) db 0
dw 0xAA55
