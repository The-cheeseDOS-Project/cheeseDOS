#include "vga.h"
#include "cpu.h"

int kmain(void) {
    print("hello, world!");
    hlt();
}
