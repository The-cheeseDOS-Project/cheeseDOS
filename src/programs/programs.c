/*
 * cheeseDOS - My x86 DOS
 * Copyright (C) 2025  Connor Thomson
 *
 * This program is free software: you can DARKREDistribute it and/or modify
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

#include "programs.h"
#include "vga.h"
#include "ramdisk.h"
#include "calc.h"
#include "string.h"
#include "rtc.h"
#include "version.h"
#include "beep.h"
#include "acpi.h"
#include "io.h"
#include "timer.h"
#include "keyboard.h"
#include "serial.h"
#include "ide.h"
#include "kernel.h"
#include "shell.h"
#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

static uint32_t current_dir_inode_no = 0;

static void copy_inode_callback(const char *name, uint32_t inode_no);

static uint8_t default_text_fg_color = COLOR_WHITE;
static uint8_t default_text_bg_color = COLOR_BLACK;

static ramdisk_inode_t *search_result = NULL;
static ramdisk_inode_t *copy_src;
static ramdisk_inode_t *copy_dst;

static const char *search_name = NULL;

static void print_uint(uint32_t num) {
    char buf[12];
    int i = 0;
    if (num == 0) {
        vga_putchar('0');
        return;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        vga_putchar(buf[j]);
    }
}

static void inode_search_callback(const char *entry_name, uint32_t inode_no) {
    if (kstrcmp(entry_name, search_name) == 0) {
        search_result = ramdisk_iget(inode_no);
    }
}

ramdisk_inode_t *ramdisk_find_inode_by_name(ramdisk_inode_t *dir, const char *name) {
    search_name = name;
    search_result = NULL;
    ramdisk_readdir(dir, inode_search_callback);
    return search_result;
}

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

typedef void (*command_func_t)(const char* args);

typedef struct {
    const char* name;
    command_func_t func;
} shell_command_t;

static void hlp(const char*) {
    print("Commands: hlp, cls, say, ver, hi, ls, see, add, rm, mkd, cd, sum, rtc, clr, ban, bep, off, res, dly, spd, run, txt, cpy, mve, die, pth, bit, svr, &, mus, ide, mem, box, hey.");
}

static void ver(const char*) {
    print("cheeseDOS version ");
    print(_binary_src_version_version_txt_start);
}

static void hi(const char*) {
    print("hello, world\n");
}

static void cls(const char*) {
    clear_screen();
}

static void say(const char* args) {
    if (args) print(args);
    print("\n");
}

static void sum(const char* args) {
    calc_command(args ? args : "");
}

static void dly(const char* args) {
    uint32_t ms = 1000;

    char buf[16] = {0};
    int i = 0;

    while (args[i] && i < 15 && args[i] >= '0' && args[i] <= '9') {
        buf[i] = args[i];
        i++;
    }

    ms = 0;
    for (i = 0; buf[i]; ++i)
        ms = ms * 10 + (buf[i] - '0');

    delay(ms);
}

void spd(const char*) {
    print("This is a deprecated Debug tool as it has no real use for what it is.\n\n");

    unsigned int start_lo, start_hi;
    __asm__ __volatile__("rdtsc" : "=a"(start_lo), "=d"(start_hi));
    unsigned int start_tsc = start_lo;

    delay(50);

    unsigned int end_lo, end_hi;
    __asm__ __volatile__("rdtsc" : "=a"(end_lo), "=d"(end_hi));
    unsigned int end_tsc = end_lo;

    unsigned int cycles = end_tsc - start_tsc;
    unsigned int mhz = cycles / 50000;

    print("~");
    print_uint(mhz);
    print(" MHz\n");
}

static void bep(const char* args) {
    int hz = 720;
    int ms = 10;

    char num1[16] = {0};
    char num2[16] = {0};
    int i = 0, j = 0;

    while (args[i] && args[i] != ' ' && i < 15) {
        num1[i] = args[i];
        i++;
    }
    if (args[i] == ' ') i++;

    while (args[i] && j < 15) {
        num2[j++] = args[i++];
    }

    hz = 0;
    for (i = 0; num1[i] >= '0' && num1[i] <= '9'; ++i)
        hz = hz * 10 + (num1[i] - '0');

    ms = 0;
    for (i = 0; num2[i] >= '0' && num2[i] <= '9'; ++i)
        ms = ms * 10 + (num2[i] - '0');

    if (hz == 0) hz = 720;
    if (ms == 0) ms = 360;

    beep(hz, ms);
}

static void off(const char*) {
    print("Shutting down... Goodbye!");
    sprint("\nShutting down target...\n\n");
    shutdown();
}

static void res(const char*) {
    print("Rebooting... See you later!");
    sprint("\nRebooting target...\n\n");
    reboot();
}

static void ls(const char* args) {
    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
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

static void see(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: see <filename>\n");
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
        print("Cannot see directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    char buf[2048];
    int read = ramdisk_readfile(file, 0, sizeof(buf) - 1, buf);
    if (read < 0) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Error reading file\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    buf[read] = 0;

    print(buf);
    print("\n");
}

static void add(const char* args) {
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

static void rm(const char* args) {
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

static void mkd(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: mkd <dirname>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    int res = ramdisk_create_dir(current_dir_inode_no, args);
    if (res == 0) {
    } else {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to create directory\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
    }
}

static void cd(const char* args) {
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

static void rtc(const char*) {
    rtc_time_t current_time;
    read_rtc_time(&current_time);
    if (current_time.month < 10) vga_putchar('0');
    print_uint(current_time.month);
    vga_putchar('/');
    if (current_time.day < 10) vga_putchar('0');
    print_uint(current_time.day);
    vga_putchar('/');
    print_uint(current_time.year);
    print(" ");
    uint8_t display_hour = current_time.hour;
    const char* ampm = "AM";
    if (display_hour >= 12) {
        ampm = "PM";
        if (display_hour > 12) {
            display_hour -= 12;
        }
    } else if (display_hour == 0) {
        display_hour = 12;
    }
    if (display_hour < 10) vga_putchar('0');
    print_uint(display_hour);
    vga_putchar(':');
    if (current_time.minute < 10) vga_putchar('0');
    print_uint(current_time.minute);
    vga_putchar(':');
    if (current_time.second < 10) vga_putchar('0');
    print_uint(current_time.second);
    vga_putchar(' ');
    print(ampm);
    print("\n");
}

static void clr(const char* arg) {
    uint8_t new_fg_color = default_text_fg_color;
    if (!arg || kstrcmp(arg, "hlp") == 0) {
        set_text_color(COLOR_DARKBLUE, COLOR_BLACK);        print("darkblue\n");
        set_text_color(COLOR_DARKGREEN, COLOR_BLACK);       print("darkgreen\n");
        set_text_color(COLOR_DARKCYAN, COLOR_BLACK);        print("darkcyan\n");
        set_text_color(COLOR_DARKRED, COLOR_BLACK);         print("darkred\n");
        set_text_color(COLOR_MAGENTA, COLOR_BLACK);         print("magenta\n");
        set_text_color(COLOR_BROWN, COLOR_BLACK);           print("brown\n");
        set_text_color(COLOR_LIGHT_GREY, COLOR_BLACK);      print("lightgrey\n");
        set_text_color(COLOR_DARK_GREY, COLOR_BLACK);       print("darkgrey\n");
        set_text_color(COLOR_BLUE, COLOR_BLACK);            print("blue\n");
        set_text_color(COLOR_GREEN, COLOR_BLACK);           print("green\n");
        set_text_color(COLOR_CYAN, COLOR_BLACK);            print("cyan\n");
        set_text_color(COLOR_RED, COLOR_BLACK);             print("red\n");
        set_text_color(COLOR_MAGENTA, COLOR_BLACK);         print("magenta\n");
        set_text_color(COLOR_YELLOW, COLOR_BLACK);          print("yellow\n");
        set_text_color(COLOR_WHITE, COLOR_BLACK);           print("white\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        print("\nUsage: clr <color>\n");
        return;
    }
    if (kstrcmp(arg, "darkblue") == 0) new_fg_color = COLOR_DARKBLUE;
    else if (kstrcmp(arg, "darkgreen") == 0) new_fg_color = COLOR_DARKGREEN;
    else if (kstrcmp(arg, "darkcyan") == 0) new_fg_color = COLOR_DARKCYAN;
    else if (kstrcmp(arg, "darkred") == 0) new_fg_color = COLOR_DARKRED;
    else if (kstrcmp(arg, "magenta") == 0) new_fg_color = COLOR_MAGENTA;
    else if (kstrcmp(arg, "brown") == 0) new_fg_color = COLOR_BROWN;
    else if (kstrcmp(arg, "lightgrey") == 0) new_fg_color = COLOR_LIGHT_GREY;
    else if (kstrcmp(arg, "darkgrey") == 0) new_fg_color = COLOR_DARK_GREY;
    else if (kstrcmp(arg, "blue") == 0) new_fg_color = COLOR_BLUE;
    else if (kstrcmp(arg, "green") == 0) new_fg_color = COLOR_GREEN;
    else if (kstrcmp(arg, "cyan") == 0) new_fg_color = COLOR_CYAN;
    else if (kstrcmp(arg, "red") == 0) new_fg_color = COLOR_RED;
    else if (kstrcmp(arg, "magenta") == 0) new_fg_color = COLOR_MAGENTA;
    else if (kstrcmp(arg, "yellow") == 0) new_fg_color = COLOR_YELLOW;
    else if (kstrcmp(arg, "white") == 0) new_fg_color = COLOR_WHITE;
    else {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Invalid color. Use 'clr hlp' for options.\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    default_text_fg_color = new_fg_color;
    default_text_bg_color = COLOR_BLACK;
    set_text_color(default_text_fg_color, default_text_bg_color);
    clear_screen();
}

void putnum(int num) {
    char buf[12];
    int i = 0;

    if (num == 0) {
        putchar('0');
        return;
    }

    if (num < 0) {
        putchar('-');
        num = -num;
    }

    while (num > 0 && i < (int)sizeof(buf)) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (i--) {
        putchar(buf[i]);
    }
}

void txt(const char *filename) {
    if (!filename || *filename == '\0') {
        putstr("Usage: txt <file>\n");
        return;
    }

    char buffer[1024] = {0};
    size_t index = 0;
    bool saved = true;
    bool is_new = false;

    uint32_t parent_inode = 0;

    clear_screen();

    ramdisk_inode_t *file = ramdisk_iget_by_name(parent_inode, filename);
    if (!file) {
        if (ramdisk_create_file(parent_inode, filename) < 0) {
            putstr("Error: Failed to create file.\n");
            return;
        }
        file = ramdisk_iget_by_name(parent_inode, filename);
        if (!file) {
            putstr("Error: File creation succeeded, but lookup failed.\n");
            return;
        }
        is_new = true;
    } else {
        memcpy(buffer, file->data, file->size);
        index = file->size;
    }

    set_text_color(COLOR_WHITE, COLOR_BLUE);
    putstr("txt [ESC = Save & Exit | INSERT = Exit (Without save)] ");
    putstr(is_new ? "[NEW FILE] " : "");
    putstr(filename);
    set_text_color(COLOR_WHITE, COLOR_BLACK); 
    
    vga_move_cursor(1, 0);

    for (size_t i = 0; i < index; i++) {
        if (buffer[i] == '\n') {
            uint8_t row, col;
            vga_get_cursor(&row, &col);
            row++;
            col = 0;
            if (row >= get_screen_height()) row = get_screen_height() - 1;
            vga_move_cursor(row, col);
        } else {
            vga_putchar(buffer[i]);
        }
    }

    while (1) {
        int ch = keyboard_getchar();

        if (ch == 27) break; 
        if (ch == KEY_INSERT) {
            saved = false;
            break;
        }

        uint8_t row, col;
        vga_get_cursor(&row, &col);

        if (ch == 8 && index > 0) {
            index--;
            for (size_t i = index; i < kstrlen(buffer); i++) {
                buffer[i] = buffer[i + 1];
            }

            uint8_t redraw_row = row;
            uint8_t redraw_col = col > 0 ? col - 1 : 0;
            if (col == 0 && row > 1) {
                redraw_row--;
                redraw_col = get_screen_width() - 1;
            }
            vga_move_cursor(redraw_row, redraw_col);

            size_t redraw_idx = index;
            uint8_t r = redraw_row, c = redraw_col;
            while (buffer[redraw_idx] != '\0') {
                if (buffer[redraw_idx] == '\n') {
                    r++;
                    c = 0;
                    if (r >= get_screen_height()) break;
                    vga_move_cursor(r, c);
                } else {
                    vga_putchar(buffer[redraw_idx]);
                    c++;
                    if (c >= get_screen_width()) {
                        c = 0;
                        r++;
                        if (r >= get_screen_height()) break;
                        vga_move_cursor(r, c);
                    }
                }
                redraw_idx++;
            }

            vga_putchar(' ');
            vga_move_cursor(redraw_row, redraw_col);
        }

        else if (ch == KEY_DELETE && index < kstrlen(buffer)) {
            for (size_t i = index; i < kstrlen(buffer); i++) {
                buffer[i] = buffer[i + 1];
            }

            size_t redraw_idx = index;
            uint8_t r = row, c = col;
            while (buffer[redraw_idx] != '\0') {
                if (buffer[redraw_idx] == '\n') {
                    r++;
                    c = 0;
                    if (r >= get_screen_height()) break;
                    vga_move_cursor(r, c);
                } else {
                    vga_putchar(buffer[redraw_idx]);
                    c++;
                    if (c >= get_screen_width()) {
                        c = 0;
                        r++;
                        if (r >= get_screen_height()) break;
                        vga_move_cursor(r, c);
                    }
                }
                redraw_idx++;
            }

            vga_putchar(' ');
            vga_move_cursor(row, col);
        }

        else if (ch == KEY_ENTER) {
            if (kstrlen(buffer) < sizeof(buffer) - 1) {
                for (size_t i = kstrlen(buffer); i >= index && i < sizeof(buffer) - 1; i--) {
                    buffer[i + 1] = buffer[i];
                }
                buffer[index++] = '\n';
                row++;
                col = 0;
                if (row >= get_screen_height()) row = get_screen_height() - 1;
                vga_move_cursor(row, col);
            }
        }

        else if (ch == KEY_HOME) {

            size_t line_start = index;
            while (line_start > 0 && buffer[line_start - 1] != '\n') {
                line_start--;
            }

            size_t target = line_start;
            while (buffer[target] == ' ' && target < kstrlen(buffer)) {
                target++;
            }
            index = target;

            uint8_t row2 = 1;
            for (size_t i = 0; i < line_start; i++) {
                if (buffer[i] == '\n') row2++;
            }
            uint8_t col2 = 0;
            for (size_t i = line_start; i < index; i++) {
                if (buffer[i] != '\n') col2++;
            }
            vga_move_cursor(row2, col2);
        }

        else if (ch == KEY_END) {

            size_t line_start = index;
            while (line_start > 0 && buffer[line_start - 1] != '\n') {
                line_start--;
            }

            size_t line_end = line_start;
            while (line_end < kstrlen(buffer) && buffer[line_end] != '\n' && buffer[line_end] != '\0') {
                line_end++;
            }

            size_t target = line_end;
            while (target > line_start && (buffer[target - 1] == ' ' || buffer[target - 1] == '\t')) {
                target--;
            }

            index = target;

            uint8_t row2 = 1;
            for (size_t i = 0; i < line_start; i++) {
                if (buffer[i] == '\n') row2++;
            }
            uint8_t col2 = 0;
            for (size_t i = line_start; i < index; i++) {
                if (buffer[i] != '\n') col2++;
            }
            vga_move_cursor(row2, col2);
        }

        else if (ch == KEY_LEFT) {
            if (index > 0) {
                index--;
                if (buffer[index] == '\n') {
                    size_t scan = index;
                    uint8_t col2 = 0;
                    while (scan > 0 && buffer[scan - 1] != '\n') {
                        scan--;
                        col2++;
                    }
                    uint8_t row2 = 1;
                    for (size_t i = 0; i < scan; i++) {
                        if (buffer[i] == '\n') row2++;
                    }
                    vga_move_cursor(row2, col2);
                } else {
                    if (col > 0) col--;
                    else if (row > 1) {
                        row--;
                        col = 0;
                        size_t scan = index;
                        while (scan > 0 && buffer[scan - 1] != '\n') {
                            scan--;
                            col++;
                        }
                    }
                    vga_move_cursor(row, col);
                }
            }
        }

        else if (ch == KEY_RIGHT) {
            if (index < kstrlen(buffer)) {
                if (buffer[index] == '\n') {
                    row++;
                    col = 0;
                    if (row >= get_screen_height()) row = get_screen_height() - 1;
                    vga_move_cursor(row, col);
                } else {
                    col++;
                    if (col >= get_screen_width()) {
                        col = 0;
                        row++;
                        if (row >= get_screen_height()) row = get_screen_height() - 1;
                    }
                    vga_move_cursor(row, col);
                }
                index++;
            }
        }

        else if (ch == KEY_UP) {
            size_t curr_start = index;
            while (curr_start > 0 && buffer[curr_start - 1] != '\n') {
                curr_start--;
            }
            if (curr_start > 0) {
                size_t prev_end = curr_start - 1;
                size_t prev_start = prev_end;
                while (prev_start > 0 && buffer[prev_start - 1] != '\n') {
                    prev_start--;
                }
                index = prev_end + 1;
                uint8_t row2 = 1;
                for (size_t i = 0; i < prev_start; i++) {
                    if (buffer[i] == '\n') row2++;
                }
                uint8_t col2 = 0;
                for (size_t i = prev_start; i <= prev_end; i++) {
                    if (buffer[i] != '\n') col2++;
                }
                vga_move_cursor(row2, col2);
            }
        }

        else if (ch == KEY_DOWN) {
            size_t line_start = index;
            while (line_start > 0 && buffer[line_start - 1] != '\n') {
                line_start--;
            }
            size_t col_target = index - line_start;
            size_t line_end = index;
            while (line_end < sizeof(buffer) && buffer[line_end] != '\n' && buffer[line_end] != '\0') {
                line_end++;
            }
            if (buffer[line_end] == '\n') {
                size_t next_start = line_end + 1;
                size_t next_end = next_start;
                while (next_end < sizeof(buffer) && buffer[next_end] != '\n' && buffer[next_end] != '\0') {
                    next_end++;
                }
                size_t next_len = next_end - next_start;
                if (col_target > next_len) col_target = next_len;
                index = next_start + col_target;
                uint8_t row2 = 1;
                for (size_t i = 0; i < index; i++) {
                    if (buffer[i] == '\n') row2++;
                }
                vga_move_cursor(row2, (uint8_t)col_target);
            }
        }

        else if (ch >= 32 && ch <= 126 && kstrlen(buffer) < sizeof(buffer) - 1) {
            for (size_t i = kstrlen(buffer); i >= index && i < sizeof(buffer) - 1; i--) {
                buffer[i + 1] = buffer[i];
            }
            buffer[index++] = (char)ch;

            uint8_t save_row = row, save_col = col;
            size_t redraw_idx = index - 1;
            uint8_t r = row, c = col;
            while (buffer[redraw_idx] != '\0' && r < get_screen_height()) {
                if (buffer[redraw_idx] == '\n') {
                    r++;
                    c = 0;
                    if (r >= get_screen_height()) break;
                    vga_move_cursor(r, c);
                } else {
                    vga_putchar(buffer[redraw_idx]);
                    c++;
                    if (c >= get_screen_width()) {
                        c = 0;
                        r++;
                        if (r >= get_screen_height()) break;
                        vga_move_cursor(r, c);
                    }
                }
                redraw_idx++;
            }
            vga_move_cursor(save_row, save_col + 1);
        }
    }

    vga_move_cursor(get_screen_height() - 1, 0);
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

static void run(const char* args) {
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

static void mve(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: mve <src> <dst>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    char src[RAMDISK_FILENAME_MAX] = {0};
    char dst[RAMDISK_FILENAME_MAX] = {0};
    int i = 0, j = 0;
    while (args[i] == ' ') i++;
    while (args[i] && args[i] != ' ' && j < RAMDISK_FILENAME_MAX - 1) {
        src[j++] = args[i++];
    }
    src[j] = '\0';
    while (args[i] == ' ') i++;
    j = 0;
    while (args[i] && j < RAMDISK_FILENAME_MAX - 1) {
        dst[j++] = args[i++];
    }
    dst[j] = '\0';

    if (src[0] == '\0' || dst[0] == '\0') {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: mve <src> <dst>\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }

    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
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
    size_t len = kstrlen(dst);
    if (len >= RAMDISK_FILENAME_MAX) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Destination name too long\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
    for (size_t k = 0; k < RAMDISK_FILENAME_MAX; ++k) src_inode->name[k] = 0;
    for (size_t k = 0; k < len; ++k) src_inode->name[k] = dst[k];
    src_inode->name[len] = 0;
}

static void copy_inode(ramdisk_inode_t *src, ramdisk_inode_t *dst) {
    if (!src || !dst) return;

    if (src->type == RAMDISK_INODE_TYPE_FILE) {
        int bytes = src->size;
        if (bytes > RAMDISK_DATA_SIZE_BYTES) bytes = RAMDISK_DATA_SIZE_BYTES;
        for (int i = 0; i < bytes; ++i) {
            dst->data[i] = src->data[i];
        }
        dst->size = bytes;
    } else if (src->type == RAMDISK_INODE_TYPE_DIR) {
        copy_src = src;
        copy_dst = dst;

        ramdisk_readdir(src, copy_inode_callback);
    }
}

static void copy_inode_callback(const char *name, uint32_t inode_no) {
    ramdisk_inode_t *child_src = ramdisk_iget(inode_no);
    if (!child_src) return;

    if (child_src->type == RAMDISK_INODE_TYPE_FILE) {
        if (ramdisk_create_file(copy_dst->inode_no, name) != 0) return;
    } else if (child_src->type == RAMDISK_INODE_TYPE_DIR) {
        if (ramdisk_create_dir(copy_dst->inode_no, name) != 0) return;
    }

    ramdisk_inode_t *child_dst = ramdisk_iget_by_name(copy_dst->inode_no, name);
    if (!child_dst) return;

    copy_inode(child_src, child_dst);
}

static void cpy(const char* args) {
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

    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
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
        ? ramdisk_create_file(current_dir_inode_no, dst)
        : ramdisk_create_dir(current_dir_inode_no, dst);

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

static unsigned int seed = 123456789;

static unsigned int rand_bare() {
    seed = (1664525 * seed + 1013904223);
    return seed;
}

static unsigned int parse_int(const char* str) {
    unsigned int result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

static void int_to_str(unsigned int value, char* buf) {
    char temp[10];
    int i = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    for (int j = 0; j < i; j++) {
        buf[j] = temp[i - j - 1];
    }
    buf[i] = '\0';
}

static void die(const char* args) {
    unsigned int max = parse_int(args);
    if (max == 0) max = 10;

    unsigned int random_number = (rand_bare() % max) + 1;

    char buf[12];
    int_to_str(random_number, buf);

    print(buf);
    print("\n");
}

static void pth(const char*) {
    char path_buffer[256];
    if (ramdisk_get_path(current_dir_inode_no, path_buffer, sizeof(path_buffer)) == 0) {
        print(path_buffer);
    } else {
        print("Error getting path\n");
    }
}

static void bit(const char* args) {
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

static void svr(const char*) {
    static uint32_t rng_state = 0x12345678;

    #define COLOR_COUNT 6
    static const uint8_t rainbow_bg[COLOR_COUNT] = {
        9, 10, 11, 12, 13, 14
    };

    uint8_t orig_row, orig_col;
    vga_get_cursor(&orig_row, &orig_col);
    vga_set_cursor(25, 0);
    for (volatile int i = 0; i < 2000000; i++);
    while (inb(0x64) & 1) inb(0x60);

    int width = get_screen_width();
    int height = get_screen_height();
    uint16_t* vga_mem = (uint16_t*)0xB8000;

    rng_state = ((rng_state >> 1) ^ (-(rng_state & 1) & 0xd0000001));
    uint8_t bg_index = (rng_state >> 4) % COLOR_COUNT;
    uint8_t bg = rainbow_bg[bg_index];
    uint8_t attr = (bg << 4) | 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pos = y * width + x;
            vga_mem[pos] = ' ' | (attr << 8);
        }
    }

    int frame_counter = 0;

    while (1) {
        if (++frame_counter >= 100) {
            if (inb(0x64) & 1) {
                inb(0x60);
                break;
            }
            frame_counter = 0;
        }

        for (int i = 0; i < 40; i++) {
            rng_state = ((rng_state >> 1) ^ (-(rng_state & 1) & 0xd0000001));
            int x = (rng_state >> 8) % width;
            int y = (rng_state >> 16) % height;
            int pos = y * width + x;

            uint8_t bg_index = (rng_state >> 4) % COLOR_COUNT;
            uint8_t bg = rainbow_bg[bg_index];
            uint8_t attr = (bg << 4) | 0;

            vga_mem[pos] = ' ' | (attr << 8);
        }

        for (volatile int delay = 0; delay < 10000; delay++);
    }

    while (inb(0x64) & 1) inb(0x60);
    vga_set_cursor(orig_row, orig_col);
    clear_screen();
    return;
}

static void box(const char*) {
    uint8_t orig_row, orig_col;
    vga_get_cursor(&orig_row, &orig_col);
    vga_set_cursor(25, 80);
    for (volatile int i = 0; i < 2000000; i++);
    while (inb(0x64) & 1) inb(0x60);

    int width = get_screen_width();
    int height = get_screen_height();
    uint16_t* vga_mem = (uint16_t*)0xB8000;

    static unsigned int seed = 1;
    seed = seed * 1103515245 + 12345;
    unsigned int rand_x = (seed / 65536) % 32768;
    seed = seed * 1103515245 + 12345;
    unsigned int rand_y = (seed / 65536) % 32768;

    const int size = 2;
    int x = size + (rand_x % (width - 2 * size));
    int y = size + (rand_y % (height - 2 * size));
    int vx = 1, vy = 1;

    uint8_t bg_colors[] = {9, 10, 11, 12, 13, 14};
    int num_colors = sizeof(bg_colors) / sizeof(bg_colors[0]);

    seed = seed * 1103515245 + 12345;
    uint8_t current_bg = bg_colors[(seed / 65536) % num_colors];
    uint8_t attr = (current_bg << 4) | 0;

    while (1) {
        if (inb(0x64) & 1) {
            inb(0x60);
            break;
        }

        clear_screen();
        x += vx;
        y += vy;
        vga_set_cursor(25, 80);

        int collided = 0;

        if (x - size <= 0 || x + size >= width) {
            vx = -vx;
            collided = 1;
            x = (x - size <= 0) ? size + 1 : width - size - 1;
        }

        if (y - size <= 0 || y + size >= height) {
            vy = -vy;
            collided = 1;
            y = (y - size <= 0) ? size + 1 : height - size - 1;
        }

        if (collided) {
            seed = seed * 1103515245 + 12345;
            uint8_t new_bg;
            do {
                new_bg = bg_colors[(seed / 65536) % num_colors];
                seed = seed * 1103515245 + 12345;
            } while (new_bg == current_bg);
            current_bg = new_bg;
            attr = (current_bg << 4) | 0;
        }

        for (int dy = -size; dy <= size; dy++) {
            for (int dx = -size - 2; dx <= size + 2; dx++) {
                int px = x + dx;
                int py = y + dy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    int pos = py * width + px;
                    vga_mem[pos] = ' ' | (attr << 8);
                }
            }
        }

        for (volatile int delay = 0; delay < 100000; delay++);
    }

    while (inb(0x64) & 1) inb(0x60);
    vga_set_cursor(orig_row, orig_col);
    clear_screen();
}

static void xmas_music() {
    print("Playing: We Wish You a Merry Christmas...");
    beep(523,386);beep(698,386);beep(698,193);beep(784,193);beep(698,193);beep(659,193);
    beep(587,386);beep(587,386);beep(587,386);
    beep(784,386);beep(784,193);beep(880,193);beep(784,193);beep(698,193);
    beep(659,386);beep(523,386);beep(523,386);
    beep(880,386);beep(880,193);beep(932,193);beep(880,193);beep(784,193);
    beep(698,386);beep(587,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(523,386);
    beep(698,386);beep(698,193);beep(784,193);beep(698,193);beep(659,193);
    beep(587,386);beep(587,386);beep(587,386);
    beep(784,386);beep(784,193);beep(880,193);beep(784,193);beep(698,193);
    beep(659,386);beep(523,386);beep(523,386);
    beep(880,386);beep(880,193);beep(932,193);beep(880,193);beep(784,193);
    beep(698,386);beep(587,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(523,386);
    beep(698,386);beep(698,386);beep(698,386);
    beep(659,771);beep(659,386);
    beep(698,386);beep(659,386);beep(587,386);
    beep(523,771);beep(880,386);
    beep(932,386);beep(880,386);beep(784,386);
    beep(1047,386);beep(523,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(523,386);
    beep(698,386);beep(698,193);beep(784,193);beep(698,193);beep(659,193);
    beep(587,386);beep(587,386);beep(587,386);
    beep(784,386);beep(784,193);beep(880,193);beep(784,193);beep(698,193);
    beep(659,386);beep(523,386);beep(523,386);
    beep(880,386);beep(880,193);beep(932,193);beep(880,193);beep(784,193);
    beep(698,386);beep(587,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(523,386);
    beep(698,386);beep(698,386);beep(698,386);
    beep(659,771);beep(659,386);
    beep(698,386);beep(659,386);beep(587,386);
    beep(523,771);beep(880,386);
    beep(932,386);beep(880,386);beep(784,386);
    beep(1047,386);beep(523,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(523,386);
    beep(698,386);beep(698,193);beep(784,193);beep(698,193);beep(659,193);
    beep(587,386);beep(587,386);beep(587,386);
    beep(784,386);beep(784,193);beep(880,193);beep(784,193);beep(698,193);
    beep(659,386);beep(523,386);beep(523,386);
    beep(880,386);beep(880,193);beep(932,193);beep(880,193);beep(784,193);
    beep(698,386);beep(587,386);beep(523,193);beep(523,193);
    beep(587,386);beep(784,386);beep(659,386);
    beep(698,771);beep(0,386);
    print(" OK!\n");
}

static void happy_birthday() {
    print("Playing: Happy Birthday...");

    beep(262,385);  beep(262,193);  beep(294,578);
    beep(262,578);  beep(349,578);  beep(330,1156);

    beep(262,385);  beep(262,193);  beep(294,578);
    beep(262,578);  beep(392,578);  beep(349,1156);

    beep(262,385);  beep(262,193);  beep(523,578);
    beep(440,578);  beep(349,578);  beep(330,578);

    beep(294,578);  beep(466,385);  beep(466,193);
    beep(440,578);  beep(349,578);  beep(392,578);

    beep(349,1156);

    print(" OK!\n");
}

static void mus(const char* args) {
    if (!args || !*args) {
        print("Usage: mus <song>\n");
        print(" Songs:\n");
        print("  birthday\n");
        print("  xmas\n");
        return;
    }

    if (kstrcmp(args, "xmas") == 0) {
        xmas_music();
    } else if (kstrcmp(args, "birthday") == 0) {
        happy_birthday();
    } else {
        print("Unknown song. Try: xmas or birthday");
    }
}

static void ide(const char*) {
    print("Master: ");
    if (ide_detect()) {
        print("True");
    } 
    else {
        print(" False\n");
    }
}

static void hey(const char* args) {
    if (!args || !*args) {
        args = "hello, world!";
    }

    int counter = 0;
    while (counter < 10000) {
        print(args);
        print(" "); // hey! dont look at me.
        counter++;
    }
}

static void mem(const char*) {
    uint32_t used = heap_ptr - (uint32_t)&_kernel_start;
    uint32_t used_kb = used / 1024;
    char buf[16];
    itoa(used_kb, buf, 10);
    print(buf);
    print("K RAM USED\n");
}

static void ban(const char*) {
    const uint8_t glyphs[][8] = {
        {0x3C,0x40,0x80,0x80,0x80,0x80,0x40,0x3C},
        {0xFC,0x82,0x81,0x81,0x81,0x81,0x82,0xFC},
        {0x7E,0x81,0x81,0x81,0x81,0x81,0x81,0x7E},
        {0x7C,0x82,0x80,0x7C,0x02,0x82,0x7C,0x00}
    };

    const int count = sizeof(glyphs) / sizeof(glyphs[0]);
    const int gw = 8, gh = 8;
    const int sw = 80, sh = 25;
    uint16_t* vga = (uint16_t*)0xB8000;

    vga_set_cursor(25, 0);

    int x[count], y[count];
    int vx[count], vy[count];
    for (int i = 0; i < count; i++) {
        x[i] = 10 + i * (gw + 2);
        y[i] = 5 + i * 2;
        vx[i] = 1;
        vy[i] = 1;
    }

    uint8_t colors[] = {
        (COLOR_BLUE << 4) | COLOR_WHITE,
        (COLOR_GREEN << 4) | COLOR_WHITE,
        (COLOR_CYAN << 4) | COLOR_WHITE,
        (COLOR_RED << 4) | COLOR_WHITE,
        (COLOR_MAGENTA << 4) | COLOR_WHITE,
        (COLOR_YELLOW << 4) | COLOR_WHITE
    };
    int color_index = 0;

    static const char* banner_top = "The cheese Diskette Operating System | Press ESCAPE (or ESC) key to exit demo. | ";
    static const char* banner_bottom =
        "cheeseDOS is an x86, fully GNU GPLed, custom C99 written, 1.44MB, monolithic, live, Single Address Space Diskette Operating System that loads into RAM. | ";
    static int scroll_top = 0;
    static int scroll_bottom = 0;

    while (1) {
        vga_set_cursor(25, 0);

        if (inb(0x64) & 1 && inb(0x60) == 0x01) break;

        for (int i = 0; i < sw * sh; i++) {
            vga[i] = ' ' | (COLOR_WHITE << 8);
        }

        for (int i = 0; i < sw; i++) {
            char ch = banner_top[(scroll_top + i) % kstrlen(banner_top)];
            vga[i] = ch | ((COLOR_MAGENTA << 4 | COLOR_WHITE) << 8);
        }
        scroll_top = (scroll_top + 1) % kstrlen(banner_top);

        for (int i = 0; i < sw; i++) {
            char ch = banner_bottom[(scroll_bottom + i) % kstrlen(banner_bottom)];
            vga[(sh - 1) * sw + i] = ch | ((COLOR_BLUE << 4 | COLOR_WHITE) << 8);
        }
        scroll_bottom = (scroll_bottom + 1) % kstrlen(banner_bottom);

        for (int i = 0; i < count; i++) {
            x[i] += vx[i];
            y[i] += vy[i];

            if (x[i] <= 0 || x[i] + gw >= sw) {
                vx[i] = -vx[i];
                color_index = (color_index + 1) % (sizeof(colors) / sizeof(colors[0]));
            }

            if (y[i] <= 1 || y[i] + gh >= sh - 1) {
                vy[i] = -vy[i];
                color_index = (color_index + 1) % (sizeof(colors) / sizeof(colors[0]));
            }

            for (int row = 0; row < gh; row++) {
                for (int col = 0; col < gw; col++) {
                    if (glyphs[i][row] & (1 << (7 - col))) {
                        int px = x[i] + col;
                        int py = y[i] + row;
                        if (px >= 0 && px < sw && py >= 1 && py < sh - 1) {
                            vga[py * sw + px] = ' ' | (colors[color_index] << 8);
                        }
                    }
                }
            }
        }

    delay(50);
    }

    clear_screen();
}

static shell_command_t commands[] = {
    {"hlp", hlp},
    {"ver", ver},
    {"hi", hi},
    {"cls", cls},
    {"say", say},
    {"sum", sum},
    {"ls", ls},
    {"see", see},
    {"rm", rm},
    {"mkd", mkd},
    {"cd", cd},
    {"rtc", rtc},
    {"clr", clr},
    {"ban", ban},
    {"bep", bep},
    {"off", off},
    {"res", res},
    {"dly", dly},
    {"spd", spd},
    {"run", run},
    {"txt", txt},
    {"mve", mve},
    {"cpy", cpy},
    {"die", die},
    {"pth", pth},
    {"bit", bit},
    {"svr", svr},
    {"mus", mus},
    {"ide", ide},
    {"hey", hey},
    {"mem", mem},
    {"box", box},
    {"add", add},
    {NULL, NULL}
};

bool execute_command(const char* command, const char* args) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (kstrcmp(command, commands[i].name) == 0) {
            commands[i].func(args);
            return true;
        }
    }

    set_text_color(COLOR_RED, COLOR_BLACK);
    sprint(" FAIL!: ");
    qprint(command);
    qprint(": command not found\n");
    set_text_color(default_text_fg_color, default_text_bg_color);

    return false;
}
