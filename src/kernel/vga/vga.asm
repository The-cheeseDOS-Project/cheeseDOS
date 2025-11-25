BITS 32

section .data
cursor_pos dd 0xB8000

section .text
global print
global putchar
global clear

print:
    mov esi, [esp + 4]
    mov ah, 0x07
.next_char:
    lodsb
    test al, al
    jz .done
    cmp al, 10
    je .newline
    mov edi, [cursor_pos]
    stosw
    add dword [cursor_pos], 2
    jmp .next_char
.newline:
    mov eax, [cursor_pos]
    sub eax, 0xB8000
    mov edx, eax
    mov ecx, 160
    div ecx
    inc eax
    mov edx, 0
    imul eax, ecx
    add eax, 0xB8000
    mov [cursor_pos], eax
    jmp .next_char
.done:
    ret

putchar:
    mov ah, 0x07
    mov al, [esp + 4]
    cmp al, 10
    je .newline_pc
    mov edi, [cursor_pos]
    stosw
    add dword [cursor_pos], 2
    ret
.newline_pc:
    mov eax, [cursor_pos]
    sub eax, 0xB8000
    mov edx, eax
    mov ecx, 160
    div ecx
    inc eax
    mov edx, 0
    imul eax, ecx
    add eax, 0xB8000
    mov [cursor_pos], eax
    ret

clear:
    mov edi, 0xB8000
    mov ecx, 80*25
    mov ah, 0x07
    mov al, ' '
.rep:
    stosw
    loop .rep
    mov dword [cursor_pos], 0xB8000
    ret
