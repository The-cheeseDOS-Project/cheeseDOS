#include "cd.h"

void cd(const char* args, uint32_t *cwd) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: cd <dirname>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    const char *dirname = args;
    if (kstrcmp(dirname, "..") == 0) {
        if (*cwd != 0) {
            ramdisk_inode_t *cur_dir = ramdisk_iget(*cwd);
            if (cur_dir) *cwd = cur_dir->parent_inode_no;
        }
        return;
    }
    ramdisk_inode_t *dir = ramdisk_iget(*cwd);
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
    *cwd = new_dir->inode_no;
}
