#include "programs.h"
#include "vga.h"
#include "calc.h"

void sum(const char* args) {
    calc_command(args ? args : "");
}