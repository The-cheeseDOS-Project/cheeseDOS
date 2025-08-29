#include "mkd.h"

void mkd(const char* args, uint32_t *cwd) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: mkd <dirname>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    int res = ramdisk_create_dir(*cwd, args);
    if (res == 0) {
    } else {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to create directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
    }
}
