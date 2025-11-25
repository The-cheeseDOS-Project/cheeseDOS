#include "vga.h"
#include "keyboard.h"
#include "cpu.h"

int kmain(void) {
    print("Kernel started!");
    keyboard_init(key_pressed);

    sti();

    while (1) {
        if (last_key) {
            char c = last_key;
            last_key = 0;
            putchar(c);
        }
    }
}
