global outb
global inb

section .text

outb:
    push ebp
    mov ebp, esp

    mov dx, [ebp+8]
    mov al, [ebp+12]
    out dx, al

    pop ebp
    ret

inb:
    push ebp
    mov ebp, esp

    mov dx, [ebp+8]
    in al, dx

    pop ebp
    ret
