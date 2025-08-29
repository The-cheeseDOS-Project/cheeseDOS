#ifndef CLK_H
#define CLK_H

#include "stdint.h"
#include "rtc.h"
#include "vga.h"



void rtc(const char* args, uint32_t *cwd);

#endif
