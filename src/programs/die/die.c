#include "programs.h"
#include "vga.h"

static unsigned int seed = 123456789;

static unsigned int rand_bare() {
    seed = (1664525 * seed + 1013904223);
    return seed;
}

static unsigned int parse_int(const char* str) {
    unsigned int result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

static void int_to_str(unsigned int value, char* buf) {
    char temp[10];
    int i = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    for (int j = 0; j < i; j++) {
        buf[j] = temp[i - j - 1];
    }
    buf[i] = '\0';
}

void die(const char* args) {
    unsigned int max = parse_int(args);
    if (max == 0) max = 10;

    unsigned int random_number = (rand_bare() % max) + 1;

    char buf[12];
    int_to_str(random_number, buf);

    print(buf);
    print("\n");
}