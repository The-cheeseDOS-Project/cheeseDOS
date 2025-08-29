#include "programs.h"
#include "vga.h"
#include "ramdisk.h"
#include "string.h"

void add(const char* args) {
    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
    ramdisk_inode_t *file = NULL;

    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: add <filename> <text_to_add>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    char filename[RAMDISK_FILENAME_MAX];
    const char *text_to_add = NULL;
    size_t args_len = kstrlen(args);
    const char *space_pos = NULL;

    for (size_t i = 0; i < args_len; ++i) {
        if (args[i] == ' ') {
            space_pos = &args[i];
            break;
        }
    }

    if (space_pos) {
        size_t filename_len = space_pos - args;
        if (filename_len >= RAMDISK_FILENAME_MAX) {
            set_text_color(COLOR_RED, COLOR_BLACK);
            print("Error: Filename too long.\n");
            set_text_color(default_text_fg_color, default_text_bg_color);
            return;
        }
        for (size_t i = 0; i < filename_len; ++i) {
            filename[i] = args[i];
        }
        filename[filename_len] = '\0';

        text_to_add = space_pos + 1;
        while (*text_to_add == ' ') {
            text_to_add++;
        }
        if (*text_to_add == '\0') {
            text_to_add = NULL;
        }
    } else {
        kstrcpy(filename, args);
        text_to_add = NULL;
    }

    if (!dir) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get current directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    file = ramdisk_find_inode_by_name(dir, filename);
    if (!file) {
        if (ramdisk_create_file(current_dir_inode_no, filename) != 0) {
            set_text_color(COLOR_RED, COLOR_BLACK);
            print("Failed to create file\n");
            set_text_color(default_text_fg_color, default_text_bg_color);
            return;
        }
        file = ramdisk_find_inode_by_name(dir, filename);
        if (!file) {
            set_text_color(COLOR_RED, COLOR_BLACK);
            print("Error: Could not retrieve newly created file.\n");
            set_text_color(default_text_fg_color, default_text_bg_color);
            return;
        }
    }

    if (file->type == RAMDISK_INODE_TYPE_DIR) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Cannot add text to a directory.\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    if (!text_to_add) {
        char empty[1] = { '\0' };
        if (ramdisk_writefile(file, 0, 0, empty) < 0) {
            set_text_color(COLOR_RED, COLOR_BLACK);
            print("Failed to write empty file\n");
            set_text_color(default_text_fg_color, default_text_bg_color);
        }
        return;
    }

    char new_content[RAMDISK_DATA_SIZE_BYTES + 1];
    size_t content_length = 0;

    int bytes_read = ramdisk_readfile(file, 0, RAMDISK_DATA_SIZE_BYTES, new_content);
    if (bytes_read > 0) {
        content_length = bytes_read;
        new_content[content_length] = '\0';

        if (content_length > 0 && new_content[content_length - 1] != '\n') {
            if (content_length < RAMDISK_DATA_SIZE_BYTES) {
                new_content[content_length++] = '\n';
                new_content[content_length] = '\0';
            }
        }
    }

    size_t text_len = kstrlen(text_to_add);
    if (content_length + text_len >= RAMDISK_DATA_SIZE_BYTES) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Error: Out of Ramdisk space.");
        print_uint(RAMDISK_DATA_SIZE_BYTES);
        print(" bytes).\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    kstrcpy(new_content + content_length, text_to_add);
    content_length += text_len;

    if (ramdisk_writefile(file, 0, content_length, new_content) < 0) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to write to file\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
}