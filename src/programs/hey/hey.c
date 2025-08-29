#include "programs.h"
#include "vga.h"

void hey(const char* args) {
    if (!args || !*args) {
        args = "hello, world!";
    }

    int counter = 0;
    while (counter < 10000) {
        print(args);
        print(" ");
        counter++;
    }
}