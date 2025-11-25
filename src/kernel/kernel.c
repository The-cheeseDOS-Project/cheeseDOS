void kmain(void) {
    unsigned int offset = (12 * 80 + 40) * 2;
    unsigned short val  = (0x0F << 8) | 0x01;

    __asm__ volatile (
        "movl $0xB8000, %%edi\n\t"
        "addl %0, %%edi\n\t"
        "movw %1, %%ax\n\t"
        "stosw\n\t"
        :
        : "r"(offset), "r"(val)
        : "ax", "edi", "memory"
    );
}
