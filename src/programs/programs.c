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
#include "banner.h"
#include "rtc.h"
#include "version.h"
#include "beep.h"
#include "acpi.h"
#include "io.h"
#include "timer.h"
#include "keyboard.h"
#include "serial.h"
#include "shell.h"
#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"

static uint32_t current_dir_inode_no = 0;
static uint8_t default_text_fg_color = COLOR_WHITE;
static uint8_t default_text_bg_color = COLOR_BLACK;

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

static ramdisk_inode_t *ramdisk_find_inode_by_name(ramdisk_inode_t *dir, const char *name) {
    ramdisk_inode_t *found = NULL;
    void callback(const char *entry_name, uint32_t inode_no) {
        if (kstrcmp(entry_name, name) == 0) {
            found = ramdisk_iget(inode_no);
        }
    }
    ramdisk_readdir(dir, callback);
    return found;
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

static void handle_rtc_command() {
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

static uint8_t ansi_to_vga_color(int ansi_color) {
    switch (ansi_color) {
        case 30: return COLOR_BLACK;
        case 31: return COLOR_DARKRED;
        case 32: return COLOR_DARKGREEN;
        case 33: return COLOR_BROWN;
        case 34: return COLOR_DARKBLUE;
        case 35: return COLOR_MAGENTA;
        case 36: return COLOR_DARKCYAN;
        case 37: return COLOR_LIGHT_GREY;
        case 90: return COLOR_DARK_GREY;
        case 91: return COLOR_RED;
        case 92: return COLOR_GREEN;
        case 93: return COLOR_YELLOW; 
        case 94: return COLOR_BLUE;
        case 95: return COLOR_MAGENTA;
        case 96: return COLOR_CYAN;
        case 97: return COLOR_WHITE;
        default: return COLOR_LIGHT_GREY;
    }
}

static uint8_t ansi_bg_to_vga_color(int ansi_color) {
    switch (ansi_color) {
        case 40: return COLOR_BLACK;
        case 41: return COLOR_DARKRED;
        case 42: return COLOR_DARKGREEN;
        case 43: return COLOR_BROWN;
        case 44: return COLOR_DARKBLUE;
        case 45: return COLOR_MAGENTA;
        case 46: return COLOR_DARKCYAN;
        case 47: return COLOR_LIGHT_GREY;
        case 100: return COLOR_DARK_GREY;
        case 101: return COLOR_RED;
        case 102: return COLOR_GREEN;
        case 103: return COLOR_YELLOW; 
        case 104: return COLOR_BLUE;
        case 105: return COLOR_MAGENTA;
        case 106: return COLOR_CYAN;
        case 107: return COLOR_WHITE;
        default: return COLOR_BLACK;
    }
}

void print_ansi(const char* ansi_str) {
    uint8_t current_fg = COLOR_WHITE;
    uint8_t current_bg = COLOR_BLACK;
    set_text_color(current_fg, current_bg);
    while (*ansi_str) {
        if (*ansi_str == '\033' && *(ansi_str + 1) == '[') {
            ansi_str += 2;
            int code_val = 0;
            int attribute_count = 0;
            int attributes[5];
            for (int i = 0; i < 5; i++) {
                attributes[i] = 0;
            }
            while (*ansi_str >= '0' && *ansi_str <= '9') {
                code_val = code_val * 10 + (*ansi_str - '0');
                ansi_str++;
            }
            if (*ansi_str == ';') {
                attributes[attribute_count++] = code_val;
                code_val = 0;
                ansi_str++;
                while (*ansi_str != 'm' && *ansi_str != '\0') {
                    if (*ansi_str >= '0' && *ansi_str <= '9') {
                        code_val = code_val * 10 + (*ansi_str - '0');
                    } else if (*ansi_str == ';') {
                        if (attribute_count < 5) attributes[attribute_count++] = code_val;
                        code_val = 0;
                    }
                    ansi_str++;
                }
                if (attribute_count < 5) attributes[attribute_count++] = code_val;
            } else if (*ansi_str == 'm') {
                if (attribute_count < 5) attributes[attribute_count++] = code_val;
            }
            if (*ansi_str == 'm') {
                for (int i = 0; i < attribute_count; i++) {
                    int attr = attributes[i];
                    if (attr == 0) {
                        current_fg = COLOR_LIGHT_GREY;
                        current_bg = COLOR_BLACK;
                    } else if (attr == 1) {
                    } else if (attr == 5) {
                    } else if (attr == 7) {
                        uint8_t temp = current_fg;
                        current_fg = current_bg;
                        current_bg = temp;
                    } else if (attr == 8) {
                        current_fg = current_bg;
                    } else if (attr >= 30 && attr <= 37) {
                        current_fg = ansi_to_vga_color(attr);
                    } else if (attr >= 40 && attr <= 47) {
                        current_bg = ansi_bg_to_vga_color(attr);
                    } else if (attr >= 90 && attr <= 97) {
                        current_fg = ansi_to_vga_color(attr);
                    } else if (attr >= 100 && attr <= 107) {
                        current_bg = ansi_bg_to_vga_color(attr);
                    } else if (attr == 25) {
                    } else if (attr == 27) {
                    } else if (attr == 28) {
                    }
                }
                set_text_color(current_fg, current_bg);
                ansi_str++;
            } else if (*ansi_str == 's') {
                ansi_str++;
            } else if (*ansi_str == 'u') {
                ansi_str++;
            } else {
                while (*ansi_str != 'm' && *ansi_str != '\0') {
                    ansi_str++;
                }
                if (*ansi_str == 'm') ansi_str++;
            }
        } else {
            vga_putchar(*ansi_str);
            ansi_str++;
        }
    }
}

void print_int(int n) {
    if (n < 0) {
        print("-");
        n = -n;
    }
    print_uint((unsigned int)n);
}

// TODO: Fix the weird stray '*'
static void ban(const char*) {
//   clear_screen();
    set_cursor_pos(0);
    print_ansi((const char*)_binary_src_banner_banner_txt_start);
    set_text_color(default_text_fg_color, default_text_bg_color);
//    int key;
//    print("\nPress 'e' to exit to shell");
//    int keyboard_getchar(void);
//
//    while (1) {
//        key = keyboard_getchar();
//        if (key == 'e' || key == 'E') {
//            clear_screen();
//            break;
//        }
//    }
}

typedef void (*command_func_t)(const char* args);

typedef struct {
    const char* name;
    command_func_t func;
} shell_command_t;

static void hlp(const char* args) {
    (void)args;
    print("Commands: hlp, cls, say, ver, hi, ls, see, add, rem, mkd, cd, sum, rtc, clr, ban, bep, off, res, dly, spd, run, txt, cop, mov, die.");
}

static void ver(const char* args) {
    (void)args;
    print("cheeseDOS version ");
    print(_binary_src_version_version_txt_start);
}

static void hi(const char* args) {
    (void)args;
    print("Hello, world!\n");
}

static void cls(const char* args) {
    (void)args;
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

void spd(const char* args) {
    (void)args;

    print("This is a W.I.P Debug tool. it's not very accurate. This is only really meant to see if a emulator is underclocking the CPU at all\n\n");

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

static void off(const char* args) {
    (void)args;
    print("Shutting down... Goodbye!");
    sprint("\nShutting down target...\n\n");
    shutdown();
}

static void res(const char* args) {
    (void)args;
    print("Rebooting... See you later!");
    sprint("\nRebooting target...\n\n");
    reboot();
}

static void ls(const char* args) {
    (void)args;
    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
    if (!dir) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to get directory inode\n");
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
            print("Error: Filename too long (max 32 characters).\n");
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
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: add <filename> <text_to_add>\n");
        print("Error: No text to add provided.\n");
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

    ramdisk_inode_t *file = ramdisk_find_inode_by_name(dir, filename);
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

    if (text_to_add) {
        size_t text_len = kstrlen(text_to_add);
        if (content_length + text_len >= RAMDISK_DATA_SIZE_BYTES) {
            set_text_color(COLOR_RED, COLOR_BLACK);
            print("Error: Combined text would exceed maximum file size (");
            print_uint(RAMDISK_DATA_SIZE_BYTES);
            print(" bytes).\n");
            set_text_color(default_text_fg_color, default_text_bg_color);
            return;
        }
        kstrcpy(new_content + content_length, text_to_add);
        content_length += text_len;
    } else {
        if (content_length == 0) {
             set_text_color(COLOR_YELLOW, COLOR_BLACK); 
             print("Warning: No text provided and file was empty. File created but no content added.\n");
             set_text_color(default_text_fg_color, default_text_bg_color);
             return;
        }
    }

    if (ramdisk_writefile(file, 0, content_length, new_content) < 0) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to write to file\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
        return;
    }
}

static void rem(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: rem <filename>\n");
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

static void rtc(const char* args) {
    (void)args;
    handle_rtc_command();
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

    putstr("[ESC = Save & Exit | INSERT = Exit (Without save)] ");
    putstr(is_new ? "[NEW FILE] " : "[EDITING] ");
    putstr(filename);

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

static void run_script(const char* args) {
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

static void mov(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: mov <src> <dst>\n");
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
        print("Usage: mov <src> <dst>\n");
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

static void cop(const char* args) {
    if (!args) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Usage: cop <src> <dst>\n");
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
        print("Usage: cop <src> <dst>\n");
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
    if (src_inode->type != RAMDISK_INODE_TYPE_FILE) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Copy only supports files\n");
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
    if (ramdisk_create_file(current_dir_inode_no, dst) != 0) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Failed to create destination file\n");
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
    int bytes = src_inode->size;
    if (bytes > RAMDISK_DATA_SIZE_BYTES) bytes = RAMDISK_DATA_SIZE_BYTES;
    for (int k = 0; k < bytes; ++k) dst_inode->data[k] = src_inode->data[k];
    dst_inode->size = bytes;
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

static shell_command_t commands[] = {
    {"hlp", hlp},
    {"ver", ver},
    {"hi", hi},
    {"cls", cls},
    {"say", say},
    {"sum", sum},
    {"ls", ls},
    {"see", see},
    {"add", add},
    {"rem", rem},
    {"mkd", mkd},
    {"cd", cd},
    {"rtc", rtc},
    {"clr", clr},
    {"ban", ban},
    {"bep", bep },
    {"off", off },
    {"res", res },
    {"dly", dly },
    {"spd", spd },
    {"run", run_script},
    {"txt", txt},
    {"mov", mov},
    {"cop", cop},
    {"die", die},
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
