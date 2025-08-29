#include "programs.h"
#include "timer.h"

void dly(const char* args) {
    uint32_t ms = 1000;

    char buf[16] = {0};
    int i = 0;

    while (args[i] && i < 15 && args[i] >= '0' && args[i] <= '9') {
        buf[i] = args[i];
        i++;
    }

    ms = 0;
    for (i = 0; buf[i]; ++i)
        ms = ms * 10 + (buf[i] - '0');

    delay(ms);
}