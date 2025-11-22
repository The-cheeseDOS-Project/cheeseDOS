ORG 0x7C00
BITS 16

start:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00

    mov ah, 0x00
    mov al, 0x03
    int 0x10

    mov si, bootmsg
    call bios_print

    mov [BootDrive], dl
    mov si, 0x8000
    mov bx, si
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [BootDrive]
    int 0x13
    cmp dword [si], 0x464C457F
    jne $

    mov eax, [si+0x18]
    mov [ElfEntry], eax
    mov eax, [si+0x1C]
    mov [PhOff], eax
    mov cx, [si+0x2C]
    mov dx, [si+0x2A]
    mov bx, si
    add bx, [PhOff]

seg_loop:
    cmp cx, 0
    je seg_done
    cmp dword [bx], 1
    jne seg_skip
    mov eax, [bx+0x08]
    mov edx, [bx+0x0C]
    mov esi, [bx+0x14]
    mov edi, eax
    mov ecx, edx
    rep movsb
seg_skip:
    add bx, dx
    dec cx
    jmp seg_loop

seg_done:
    push ds
    lgdt [gdtinfo]

    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pmode

[BITS 32]
pmode:
    mov bx, 0x10
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    mov ss, bx

    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    jmp 0x0:huge_unreal

[BITS 16]
huge_unreal:
    pop ds
    xor ax, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    sti

    mov si, success_msg
    call bios_print

    mov eax, [ElfEntry]
    jmp eax

bios_print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp bios_print
.done:
    ret

bootmsg:
    db "Starting cheeseDOS...",0x0D,0x0A,0

success_msg:
    db "Hello, unreal mode!",0x0D,0x0A,0

BootDrive db 0
ElfEntry dd 0
PhOff dd 0

align 8
gdt:
    dd 0,0

flatcode:
    db 0xff,0xff,0,0,0,10011010b,10001111b,0

flatdata:
    db 0xff,0xff,0,0,0,10010010b,11001111b,0

gdt_end:

gdtinfo:
    dw gdt_end - gdt - 1
    dd gdt

times 510-($-$$) db 0
dw 0xAA55
