#include "programs.h"
#include "vga.h"
#include "ramdisk.h"

void bit(const char* args) {
    if (!args || *args == '\0') {
        print("Usage: bit <filename>\n");
        return;
    }

    while (*args == ' ') args++;

    const char *filename = args;

    int filename_len = 0;
    while (filename[filename_len] && filename[filename_len] != ' ') {
        filename_len++;
    }

    char fname[64];
    if (filename_len >= 64) {
        print("Filename too long\n");
        return;
    }

    for (int i = 0; i < filename_len; i++) {
        fname[i] = filename[i];
    }
    fname[filename_len] = '\0';

    ramdisk_inode_t *file = ramdisk_iget_by_name(0, fname);

    if (!file) {
        print("File not found: ");
        print(fname);
        print("\n");
        return;
    }

    if (file->type == RAMDISK_INODE_TYPE_DIR) {
        print(fname);
        print(" is a directory\n");
        return;
    }

    if (file->type != RAMDISK_INODE_TYPE_FILE) {
        print(fname);
        print(" is not a regular file\n");
        return;
    }

    uint32_t size = file->size;
    if (size == 0) {
        print("0");
    } else {
        char size_str[16];
        int pos = 0;

        while (size > 0) {
            size_str[pos++] = '0' + (size % 10);
            size /= 10;
        }

        for (int i = pos - 1; i >= 0; i--) {
            char c = size_str[i];
            vga_putchar(c);
        }
    }

    print("B\n");
}