#include "beep.h"

void bep(const char* args, uint32_t *cwd) {
    int hz = 720;
    int ms = 10;

    char num1[16] = {0};
    char num2[16] = {0};
    int i = 0, j = 0;

    while (args[i] && args[i] != ' ' && i < 15) {
        num1[i] = args[i];
        i++;
    }
    if (args[i] == ' ') i++;

    while (args[i] && j < 15) {
        num2[j++] = args[i++];
    }

    hz = 0;
    for (i = 0; num1[i] >= '0' && num1[i] <= '9'; ++i)
        hz = hz * 10 + (num1[i] - '0');

    ms = 0;
    for (i = 0; num2[i] >= '0' && num2[i] <= '9'; ++i)
        ms = ms * 10 + (num2[i] - '0');

    if (hz == 0) hz = 720;
    if (ms == 0) ms = 360;

    beep(hz, ms);
}
