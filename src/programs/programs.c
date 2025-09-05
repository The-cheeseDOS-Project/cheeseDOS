/*
 * cheeseDOS - My x86 DOS
 * Copyright (C) 2025  Connor Thomson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "vga.h"
#include "string.h"
#include "serial.h"
#include "programs.h"
#include "stdbool.h"
#include "ver.h"
#include "hlp.h"
#include "ls.h"
#include "hi.h"
#include "cls.h"
#include "say.h"
#include "sum.h"
#include "dly.h"
#include "bep.h"
#include "off.h"
#include "res.h"
#include "see.h"
#include "add.h"
#include "rm.h"
#include "txt.h"
#include "mkd.h"
#include "ban.h"
#include "cd.h"
#include "clk.h"
#include "clr.h"
#include "cpy.h"
#include "mem.h"
#include "mus.h"
#include "box.h"
#include "svr.h"
#include "bit.h"
#include "mve.h"
#include "run.h"
#include "hey.h"
#include "die.h"
#include "dve.h"
#include "pth.h"
#include "snk.h"
#include "key.h"
#include "dmp.h"
#include "rep.h"

typedef struct {
    const char* name;
    void (*func)(const char* args);
} shell_command_t;

static shell_command_t commands[] = {
    {"hlp", hlp},
    {"ver", ver},
    {"hi", hi},
    {"cls", cls},
    {"say", say},
    {"sum", sum},
    {"ls", ls},
    {"see", see},
    {"rm", rm},
    {"mkd", mkd},
    {"cd", cd},
    {"clk", clk},
    {"clr", clr},
    {"ban", ban},
    {"bep", bep},
    {"off", off},
    {"res", res},
    {"dly", dly},
    {"run", run},
    {"txt", txt},
    {"mve", mve},
    {"cpy", cpy},
    {"die", die},
    {"pth", pth},
    {"bit", bit},
    {"svr", svr},
    {"mus", mus},
    {"dve", dve},
    {"hey", hey},
    {"mem", mem},
    {"box", box},
    {"add", add},
    {"snk", snk},
    {"key", key},
    {"dmp", dmp},
    {"rep", rep},
    {NULL, NULL}
};

bool execute_command(const char* command, const char* args) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (kstrcmp(command, commands[i].name) == 0) {
            commands[i].func(args);
            return true;
        }
    }

    set_text_color(COLOR_RED, COLOR_BLACK);
    sprint(" FAIL!: ");
    qprint(command);
    qprint(": command not found\n");
    set_text_color(default_text_fg_color, default_text_bg_color);

    return false;
}
