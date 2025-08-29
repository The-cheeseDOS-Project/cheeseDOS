#include "cpy.h"

void cpy(const char* args, uint32_t *cwd) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: cpy <src> <dst>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    char src[RAMDISK_FILENAME_MAX] = {0};
    char dst[RAMDISK_FILENAME_MAX] = {0};
    int i = 0, j = 0;
    while (args[i] == ' ') i++;
    while (args[i] && args[i] != ' ' && j < RAMDISK_FILENAME_MAX - 1) src[j++] = args[i++];
    src[j] = '\0';

    while (args[i] == ' ') i++;
    j = 0;
    while (args[i] && j < RAMDISK_FILENAME_MAX - 1) dst[j++] = args[i++];
    dst[j] = '\0';

    if (src[0] == '\0' || dst[0] == '\0') {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: cpy <src> <dst>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    ramdisk_inode_t *dir = ramdisk_iget(*cwd);
    if (!dir) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get current directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    ramdisk_inode_t *src_inode = ramdisk_find_inode_by_name(dir, src);
    if (!src_inode) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Source not found\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    ramdisk_inode_t *dst_inode = ramdisk_find_inode_by_name(dir, dst);
    if (dst_inode) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Destination already exists\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    int create_result = (src_inode->type == RAMDISK_INODE_TYPE_FILE)
        ? ramdisk_create_file(*cwd, dst)
        : ramdisk_create_dir(*cwd, dst);

    if (create_result != 0) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to create destination\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    dst_inode = ramdisk_find_inode_by_name(dir, dst);
    if (!dst_inode) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get destination inode\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    copy_inode(src_inode, dst_inode);
}
