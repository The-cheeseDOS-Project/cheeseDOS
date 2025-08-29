#include "programs.h"
#include "vga.h"
#include "ramdisk.h"

void rm(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: rm <filename>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    int res = ramdisk_remove_file(current_dir_inode_no, args);
    if (res == 0) {
    } else {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to remove file\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
    }
}