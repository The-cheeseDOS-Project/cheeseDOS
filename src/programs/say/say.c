#include "say.h"

void say(const char* args, uint32_t *cwd) {
    if (args) print(args);
    print("\n");
}
