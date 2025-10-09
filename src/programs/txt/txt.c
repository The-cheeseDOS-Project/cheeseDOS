/*
 * cheeseDOS - My x86 DOS
 * Copyright (C) 2025  Connor Thomson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "vga.h"
#include "ramdisk.h"
#include "keyboard.h"
#include "string.h"
#include "stdbool.h"

#define TXT_BUFFER_SIZE 1024

static void insert_char(char *buffer, size_t *index, int ch) {
    size_t len = kstrlen(buffer);
    if (len >= TXT_BUFFER_SIZE - 1) return;

    for (size_t i = len; i > *index; i--) {
        buffer[i] = buffer[i - 1];
    }
    buffer[*index] = (char)ch;
    (*index)++;
    buffer[len + 1] = '\0';
}

static void handle_delete(char *buffer, size_t *index) {
    if (*index < kstrlen(buffer)) {
        for (size_t i = *index; i < kstrlen(buffer); i++) {
            buffer[i] = buffer[i + 1];
        }
    }
}

static void handle_backspace(char *buffer, size_t *index) {
    if (*index > 0) {
        (*index)--;
        for (size_t i = *index; i < kstrlen(buffer); i++) {
            buffer[i] = buffer[i + 1];
        }
    }
}

static void handle_left_arrow(size_t *index, const char *buffer) {
    (void)buffer;
    if (*index > 0) {
        (*index)--;
    }
}

static void handle_right_arrow(size_t *index, const char *buffer) {
    (void)buffer;
    if (*index < kstrlen(buffer)) {
        (*index)++;
    }
}

static void handle_home(size_t *index, const char *buffer) {
    (void)buffer;
    size_t line_start = *index;
    while (line_start > 0 && buffer[line_start - 1] != '\n') {
        line_start--;
    }
    *index = line_start;
}

static void handle_end(size_t *index, const char *buffer) {
    (void)buffer;
    size_t line_end = *index;
    while (line_end < kstrlen(buffer) && buffer[line_end] != '\n' && buffer[line_end] != '\0') {
        line_end++;
    }
    *index = line_end;
}

static void handle_up_arrow(size_t *index, const char *buffer) {
    (void)buffer;
    size_t line_start = *index;
    while (line_start > 0 && buffer[line_start - 1] != '\n') {
        line_start--;
    }
    if (line_start > 0) {
        size_t prev_end = line_start - 1;
        size_t prev_start = prev_end;
        while (prev_start > 0 && buffer[prev_start - 1] != '\n') {
            prev_start--;
        }
        size_t col_target = *index - line_start;
        size_t prev_len = prev_end - prev_start;
        if (col_target > prev_len) {
            col_target = prev_len;
        }
        *index = prev_start + col_target;
    }
}

static void handle_down_arrow(size_t *index, const char *buffer) {
    (void)buffer;
    size_t line_end = *index;
    while (line_end < kstrlen(buffer) && buffer[line_end] != '\n' && buffer[line_end] != '\0') {
        line_end++;
    }
    if (buffer[line_end] == '\n') {
        size_t next_start = line_end + 1;
        size_t next_end = next_start;
        while (next_end < kstrlen(buffer) && buffer[next_end] != '\n' && buffer[next_end] != '\0') {
            next_end++;
        }
        size_t next_len = next_end - next_start;
        size_t col_target = *index - (line_end - (kstrlen(buffer) - *index) );
        if (col_target > next_len) {
            col_target = next_len;
        }
        *index = next_start + col_target;
    }
}

static ramdisk_inode_t* init_editor(const char *filename, char *buffer, size_t *index, bool *is_new) {
    uint32_t parent_inode = current_dir_inode_no;
    ramdisk_inode_t *file = ramdisk_iget_by_name(parent_inode, filename);
    if (!file) {
        if (ramdisk_create_file(parent_inode, filename) < 0) {
            print("Error: Failed to create file.\n");
            return NULL;
        }
        file = ramdisk_iget_by_name(parent_inode, filename);
        if (!file) {
            print("Error: File creation succeeded, but lookup failed.\n");
            return NULL;
        }
        *is_new = true;
    } else {
        size_t copy_bytes = file->size;
        if (copy_bytes > TXT_BUFFER_SIZE - 1) copy_bytes = TXT_BUFFER_SIZE - 1;
        memcpy(buffer, file->data, copy_bytes);
        buffer[copy_bytes] = '\0';
        *index = copy_bytes;
    }
    return file;
}

static void setup_ui(const char *filename, bool is_new, const char* buffer, size_t index) {
    clear_screen();
    set_text_color(COLOR_WHITE, COLOR_BLUE);
    print("txt [ESC = Save & Exit | INSERT = Exit (Without save)] ");
    print(is_new ? "[NEW FILE] " : "");
    print(filename);
    set_text_color(COLOR_WHITE, COLOR_BLACK);
    move_cursor(1, 0);

    for (size_t i = 0; buffer[i] != '\0' && i < TXT_BUFFER_SIZE; i++) {
        putchar(buffer[i]);
    }

    uint8_t row = 1, col = 0;
    for (size_t i = 0; i < index; i++) {
        if (buffer[i] == '\n') {
            row++;
            col = 0;
        } else {
            col++;
            if (col >= get_screen_width()) {
                row++;
                col = 0;
            }
        }
    }
    move_cursor(row, col);
}

void txt(const char *filename) {
    if (!filename || *filename == '\0') {
        print("Usage: txt <file>\n");
        return;
    }

    char buffer[TXT_BUFFER_SIZE];
    size_t index = 0;
    bool saved = true;
    bool is_new = false;

    ramdisk_inode_t *file = init_editor(filename, buffer, &index, &is_new);
    if (file == NULL) {
        return;
    }

    setup_ui(filename, is_new, buffer, index);
    move_cursor(1, 0);

    uint8_t row = 1, col = 0;
    for (size_t i = 0; i < index; i++) {
        if (buffer[i] == '\n') {
            row++;
            col = 0;
        } else {
            col++;
            if (col >= get_screen_width()) {
                row++;
                col = 0;
            }
        }
    }
    move_cursor(row, col);

    while (1) {
        int ch = keyboard_getchar();
        bool needs_redraw = false;

        switch (ch) {
            case 27:
                goto end_loop;
            case KEY_INSERT:
                saved = false;
                goto end_loop;
            case 8:
                handle_backspace(buffer, &index);
                needs_redraw = true;
                break;
            case KEY_DELETE:
                handle_delete(buffer, &index);
                needs_redraw = true;
                break;
            case KEY_ENTER:
                insert_char(buffer, &index, '\n');
                needs_redraw = true;
                break;
            case KEY_LEFT:
                handle_left_arrow(&index, buffer);
                break;
            case KEY_RIGHT:
                handle_right_arrow(&index, buffer);
                break;
            case KEY_HOME:
                handle_home(&index, buffer);
                break;
            case KEY_END:
                handle_end(&index, buffer);
                break;
            case KEY_UP:
                handle_up_arrow(&index, buffer);
                break;
            case KEY_DOWN:
                handle_down_arrow(&index, buffer);
                break;
            default:
                if (ch >= 32 && ch <= 126) {
                    insert_char(buffer, &index, ch);
                    needs_redraw = true;
                }
                break;
        }

        if (needs_redraw) {
            setup_ui(filename, is_new, buffer, index);
        } else {

            uint8_t row = 1, col = 0;
            for (size_t i = 0; i < index; i++) {
                if (buffer[i] == '\n') {
                    row++;
                    col = 0;
                } else {
                    col++;
                    if (col >= get_screen_width()) {
                        row++;
                        col = 0;
                    }
                }
            }
            move_cursor(row, col);
        }
    }

end_loop:
    move_cursor(get_screen_height() - 1, 0);
    if (saved) {
        memcpy(file->data, buffer, index);
        file->size = index;
        clear_screen();
        print("Saved as: ");
        print(filename);
        print("\n");
    } else {
        clear_screen();
        print("File was not saved.\n");
    }
}
