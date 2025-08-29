#include "ls.h"

static void print_name_callback(const char *name, uint32_t inode) {
    if (kstrcmp(name, "/") == 0) return;
    ramdisk_inode_t *node = ramdisk_iget(inode);
    if (node && node->type == RAMDISK_INODE_TYPE_DIR) {
        print("[");
        print(name);
        print("]\n");
    } else {
        print(name);
        print("\n");
    }
}

void ls(const char* args, uint32_t *cwd) {
    ramdisk_inode_t *dir = ramdisk_iget(*cwd);
    if (!dir) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get directory inode\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    if (dir->type != RAMDISK_INODE_TYPE_DIR) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Not a directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    ramdisk_readdir(dir, print_name_callback);
}
