#include "programs.h"
#include "vga.h"
#include "ide.h"

void dve(const char*) {
    print("Master: ");
    if (ide_detect()) {
        print("True");
    } 
    else {
        print(" False\n");
    }
}