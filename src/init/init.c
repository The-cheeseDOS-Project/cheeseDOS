void kmain(void) {
    __asm volatile (
        "movl $msg, %esi\n"
        "call print_string\n"
        "jmp .\n"

        "print_string:\n"
        "movl %esi, %edi\n"
        ".next_char:\n"
        "movb (%edi), %al\n"
        "cmpb $0, %al\n"
        "je .done\n"
        "movb $0x0E, %ah\n"
        "int $0x10\n"
        "incl %edi\n"
        "jmp .next_char\n"
        ".done:\n"
        "cli\n"
        "hlt\n"

        "msg:\n"
        ".asciz \"Hello World\""
    );
}
