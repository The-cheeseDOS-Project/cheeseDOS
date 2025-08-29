#include "dve.h"

void dve(const char*, uint32_t *cwd) {
    print("Master: ");
    if (ide_detect()) {
        print("True");
    } 
    else {
        print(" False\n");
    }
}
