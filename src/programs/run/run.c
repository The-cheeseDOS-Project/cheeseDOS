#include "programs.h"
#include "vga.h"
#include "ramdisk.h"
#include "shell.h"
#include "string.h"

void run(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: run <filename>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    
    const char *filename = args;
    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
    if (!dir) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get current directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    ramdisk_inode_t *file = ramdisk_find_inode_by_name(dir, filename);
    if (!file) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("File not found\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    if (file->type == RAMDISK_INODE_TYPE_DIR) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Cannot run a directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    
    char buf[RAMDISK_DATA_SIZE_BYTES + 1];
    int read = ramdisk_readfile(file, 0, sizeof(buf) - 1, buf);
    if (read < 0) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Error reading file\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    buf[read] = '\0';
    
    char *script_line = buf;
    char *next_line;
    while ((next_line = (char*)kstrchr(script_line, '\n')) != NULL) {
        *next_line = '\0';
        shell_execute(script_line);
        script_line = next_line + 1;
    }
    if (*script_line != '\0') {
        shell_execute(script_line);
    }
}