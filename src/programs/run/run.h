#ifndef RUN_H
#define RUN_H

#include "vga.h"
#include "ramdisk.h"
#include "shell.h"
#include "string.h"

void run(const char* args, uint32_t *cwd);

#endif
