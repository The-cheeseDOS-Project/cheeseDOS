#include "programs.h"
#include "ramdisk.h"
#include "vga.h"
#include "string.h"

void cd(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: cd <dirname>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    const char *dirname = args;
    if (kstrcmp(dirname, "..") == 0) {
        if (current_dir_inode_no != 0) {
            ramdisk_inode_t *cur_dir = ramdisk_iget(current_dir_inode_no);
            if (cur_dir) current_dir_inode_no = cur_dir->parent_inode_no;
        }
        return;
    }
    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
    if (!dir) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get current directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    ramdisk_inode_t *new_dir = ramdisk_find_inode_by_name(dir, dirname);
    if (!new_dir || new_dir->type != RAMDISK_INODE_TYPE_DIR) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Directory not found\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    current_dir_inode_no = new_dir->inode_no;
}