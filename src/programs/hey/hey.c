#include "hey.h"

void hey(const char* args, uint32_t *cwd) {
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
