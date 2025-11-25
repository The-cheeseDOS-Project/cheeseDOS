#include "vga.h"
#include "cpu.h"

int kmain(void) {
    print("Kernel started!");
    hlt();
}
