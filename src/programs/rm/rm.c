#include "rm.h"

void rm(const char* args, uint32_t *cwd) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: rm <filename>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    int res = ramdisk_remove_file(*cwd, args);
    if (res == 0) {
    } else {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to remove file\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
    }
}
