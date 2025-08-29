#include "programs.h"
#include "vga.h"

void say(const char* args) {
    if (args) print(args);
    print("\n");
}