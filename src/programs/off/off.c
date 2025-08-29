#include "off.h"

void off(const char *args, uint32_t *cwd) {
    shutdown();
}
