#include "vga.h"
#include "cpu.h"
#include "info.h"

int kmain(void) {
    clear();
    print("Built with: ");
    print(CC);
    print(" at ");
    print(BUILD_TIME);
    print(" on ");
    print(BUILD_DATE);
    print("\n");
    hlt();
}
