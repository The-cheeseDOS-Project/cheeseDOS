ORG 0x7E00
BITS 16
stage2:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    mov ah, 0x02
    mov bh, 0x00
    mov dh, 0x02
    mov dl, 0x00
    int 0x10
    
    push ds
    mov si, loading_gdt
    call print
    lgdt [gdtinfo]
    mov si, enter_pmode
    call print
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
    mov si, entering_unreal
    call print
    mov si, hello_msg
    call print
    jmp $
    
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

loading_gdt db "Loading GDT...", 0x0D, 0x0A, 0
enter_pmode db "Entering Protected Mode...", 0x0D, 0x0A, 0
entering_unreal db "Entering Unreal mode...", 0x0D, 0x0A, 0
hello_msg db "hello, world!",0x0D,0x0A,0
    
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
