#include "mem.h"

static void ram_used() {
    uint32_t used = heap_ptr - (uint32_t)&_kernel_start;
    uint32_t used_kb = used / 1024;
    char buf[16];
    itoa(used_kb, buf, 10);
    print(buf);
    print("K");
}

static void ram_allocated() {
    unsigned char low, high;
    unsigned short ext_kb, total_kb, total_mb;

    outb(0x70, 0x30); low = inb(0x71);
    outb(0x70, 0x31); high = inb(0x71);

    ext_kb = ((unsigned short)high << 8) | low;
    total_kb = 640 + ext_kb;
    total_mb = total_kb / 1024;

    unsigned short val = total_kb;
    unsigned short div = 10000;
    int started = 0;
    while (div > 0) {
        char digit = (val / div) % 10;
        if (digit || started || div == 1) {
            char c[2] = { '0' + digit, '\0' };
            print(c);
            started = 1;
        }
        div /= 10;
    }

    print("K");
}

void mem(const char*, uint32_t *cwd) {
    ram_used();
    print(" / ");
    ram_allocated();
    print("\n");
}
