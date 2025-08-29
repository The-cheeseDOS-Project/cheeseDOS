#ifndef CD_H
#define CD_H

#include "ramdisk.h"
#include "vga.h"
#include "stdint.h"
#include "string.h"

void cd(const char* args, uint32_t *cwd);

#endif
