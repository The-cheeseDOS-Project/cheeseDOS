ORG 0x7E00
BITS 16
stage2:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [BootDrive], dl
    
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
    jmp 0x0:unreal
    
[BITS 16]
unreal:
    pop ds
    xor ax, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    sti
    mov si, entering_unreal
    call print
    jmp boot
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

boot:
    mov si, loading_disk
    call print

    mov bx, 0
    mov ax, 0x1000
    mov es, ax
    mov dl, [BootDrive]
    mov dh, 0
    mov ch, 0
    mov cl, 2

read:
    mov ah, 2
    mov al, 1
    int 0x13
    jc done
    mov ax, [es:bx]
    cmp ax, 0
    je done
    add bx, 512
    inc cl
    jmp read

done:
    mov si, hello_msg
    call print

halt:
    cli
    hlt

.ret:
    ret

loading_disk db "Loading kernel...",0x0D, 0x0A, 0
loading_gdt db "Loading GDT...", 0x0D, 0x0A, 0
enter_pmode db "Entering Protected Mode...", 0x0D, 0x0A, 0
entering_unreal db "Entering Unreal mode...", 0x0D, 0x0A, 0
hello_msg db "hello, world!",0x0D,0x0A,0

BootDrive db 0

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
