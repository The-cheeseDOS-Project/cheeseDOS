BITS 32

section .text
global print
global putchar

print:
    mov edi, 0xB8000 + 9*160
    mov esi, [esp + 4]
    mov ah, 0x07
.next_char:
    lodsb
    test al, al
    jz .done
    cmp al, 10
    je .newline
    stosw
    jmp .next_char
.newline:
    add edi, 160 - 2
    jmp .next_char
.done:
    ret

putchar:
    mov edi, 0xB8000 + 9*160
    mov ah, 0x07
    mov al, [esp + 4]
    cmp al, 10
    je .newline_pc
    stosw
    ret
.newline_pc:
    add edi, 160
    ret
