ORG 0x7C00
BITS 16

SECT_SIZE equ 512
SECT_SHIFT equ 9
GEOM_SECTORS equ 18
GEOM_CYLINDERS equ 80
GEOM_HEADS equ 2

elf_phoff equ 0x1C
elf_phnum equ 0x2C
elf_entry equ 0x18

elf_seg_type equ 0x00
elf_seg_file_offset equ 0x04
elf_seg_paddr equ 0x0C
elf_seg_filesz equ 0x10
elf_seg_memsz equ 0x14
elf_seg_struct_size equ 0x20

gdt_fl_pglimit equ 0x80
gdt_fl_32b equ 0x40

gdt_a_present equ 0x80
gdt_a_dpl0 equ 0x00
gdt_a_nosys equ 0x10
gdt_a_exec equ 0x08
gdt_a_conforming equ 0x04
gdt_a_grow_down equ 0x04
gdt_a_rw equ 0x2
gdt_a_accessed equ 0x1

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
    jc no_drives

    cmp dword [si], 0x464C457F
    jne no_drives

    jmp load_init

load_init:
    mov si, 0x8000
    mov eax, [si+0x18]
    mov [ElfEntry], eax
    mov eax, [si+0x1C]
    mov [PhOff], eax
    mov cx, [si+0x2C]
    mov dx, [si+0x2A]

load_segment:
    push cx
    mov bx, si
    add bx, [PhOff]
    
    cmp dword [bx], 1
    jne skip_seg

    mov eax, [bx+0x04]
    shr eax, SECT_SHIFT
    inc ax

    push dx
    xor dx, dx
    push bx
    mov bx, GEOM_SECTORS
    div bx
    mov cl, dl
    inc cl
    mov bl, GEOM_HEADS
    div bl
    pop bx
    mov dh, ah
    mov ch, al
    pop dx

    mov eax, [bx+0x10]
    add eax, SECT_SIZE - 1
    shr eax, SECT_SHIFT

    push bx
    mov ebx, [bx+0x0C]
    shr ebx, 4
    mov es, bx
    xor bx, bx
    mov ah, 0x02
    mov dl, [BootDrive]
    int 0x13
    pop bx
    jc load_seg_fail

skip_seg:
    mov eax, [PhOff]
    add ax, dx
    mov [PhOff], eax
    pop cx
    loop load_segment

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

no_drives:
    mov si, msg_no_drives
    call bios_print
    cli
    hlt

load_seg_fail:
    mov si, msg_load_seg_fail
    call bios_print
    cli
    hlt

bootmsg:
    db "Starting cheeseDOS...",0x0D,0x0A,0

success_msg:
    db "Hello, unreal mode!",0x0D,0x0A,0

msg_no_drives:
    db "Error: cheeseDOS could not find any bootable devices!",0x0D,0x0A,0

msg_load_seg_fail:
    db "Error: cheeseDOS failed to load one or more segments!",0x0D,0x0A,0

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
