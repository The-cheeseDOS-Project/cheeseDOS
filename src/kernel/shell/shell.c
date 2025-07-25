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

#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "ramdisk.h"
#include "calc.h"
#include "string.h"
#include "banner.h"
#include "rtc.h"
#include <stddef.h>
#include <stdint.h>

#define INPUT_BUF_SIZE 256
#define HISTORY_SIZE 32

static int prompt_start_vga_pos;
static char history[HISTORY_SIZE][INPUT_BUF_SIZE];
static int history_count = 0;
static int history_pos = 0;
static int history_view_pos = -1;
static uint32_t current_dir_inode_no = 0;

static uint8_t default_text_fg_color = COLOR_WHITE;
static uint8_t default_text_bg_color = COLOR_BLACK;

static void print_uint(uint32_t num) {
    char buf[12];
    int i = 0;
    if (num == 0) {
        putchar('0');
        return;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        putchar(buf[j]);
    }
}

static void print_prompt() {
    set_text_color(COLOR_YELLOW, COLOR_BLACK); 
    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
    if (dir) {
        char path[INPUT_BUF_SIZE];
        if (ramdisk_get_path(current_dir_inode_no, path, INPUT_BUF_SIZE) == 0) {
            print(path);
        } else {
            print("/");
        }
    } else {
        print("/");
    }
    set_text_color(COLOR_CYAN, COLOR_BLACK);
    print("> ");
    set_text_color(default_text_fg_color, default_text_bg_color);
}

static void add_history(const char *cmd) {
    if (cmd[0] == '\0') return;
    if (history_count < HISTORY_SIZE) {
        kstrncpy(history[history_count], cmd, INPUT_BUF_SIZE - 1);
        history[history_count][INPUT_BUF_SIZE - 1] = '\0';
        history_count++;
    } else {
        for (int i = 1; i < HISTORY_SIZE; i++) {
            kstrcpy(history[i - 1], history[i]);
        }
        kstrncpy(history[HISTORY_SIZE - 1], cmd, INPUT_BUF_SIZE - 1);
        history[HISTORY_SIZE - 1][INPUT_BUF_SIZE - 1] = '\0';
    }
    history_pos = history_count;
    history_view_pos = -1;
}

static void clear_input_line(int len) {
    vga_clear_chars(prompt_start_vga_pos, len);
    set_cursor_pos(prompt_start_vga_pos);
}

static void load_history_line(char *input, int *idx, int *cursor_index, int pos) {
    if (pos < 0 || pos >= history_count) return;
    clear_input_line(get_screen_width());
    kstrncpy(input, history[pos], INPUT_BUF_SIZE - 1);
    input[INPUT_BUF_SIZE - 1] = '\0';
    *idx = kstrlen(input);
    *cursor_index = *idx;
    set_cursor_pos(prompt_start_vga_pos);
    print(input);
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
    if (current_time.month < 10) putchar('0');
    print_uint(current_time.month);
    putchar('/');
    if (current_time.day < 10) putchar('0');
    print_uint(current_time.day);
    putchar('/');
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
    if (display_hour < 10) putchar('0');
    print_uint(display_hour);
    putchar(':');
    if (current_time.minute < 10) putchar('0');
    print_uint(current_time.minute);
    putchar(':');
    if (current_time.second < 10) putchar('0');
    print_uint(current_time.second);
    putchar(' ');
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

static void print_ansi(const char* ansi_str) {
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
            putchar(*ansi_str);
            ansi_str++;
        }
    }
}

static void ban(const char* args) {
    (void)args;
    clear_screen();
    set_cursor_pos(0);
    print_ansi((const char*)_binary_src_banner_banner_txt_start);
    set_text_color(default_text_fg_color, default_text_bg_color);
    int key;
    print("\nPress 'e' to exit to shell");
    while (1) {
        key = keyboard_getchar();
        if (key == 'e' || key == 'E') {
            clear_screen();
            break;
        }
    }
}

typedef void (*command_func_t)(const char* args);

typedef struct {
    const char* name;
    command_func_t func;
} shell_command_t;

static void hlp(const char* args) {
    (void)args;
    print("Commands: hlp, cls, say, ver, hi, ls, see, add, rem, mkd, cd, sum, rtc, clr, ban");
}

static void ver(const char* args) {
    (void)args;
    print("cheeseDOS alpha\n");
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
            print("Error: Filename too long (max 27 characters).\n");
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

    print("Text successfully added to file\n");
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
        print("File removed\n");
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
        print("Directory created\n");
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
        print("Moved up\n");
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
    if (kstrcmp(arg, "DARKBLUE") == 0) new_fg_color = COLOR_DARKBLUE;
    else if (kstrcmp(arg, "darkgreen") == 0) new_fg_color = COLOR_DARKGREEN;
    else if (kstrcmp(arg, "darkcyan") == 0) new_fg_color = COLOR_DARKCYAN;
    else if (kstrcmp(arg, "darkred") == 0) new_fg_color = COLOR_DARKRED;
    else if (kstrcmp(arg, "magenta") == 0) new_fg_color = COLOR_MAGENTA;
    else if (kstrcmp(arg, "yellow") == 0) new_fg_color = COLOR_YELLOW;
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
    print("Color set.\n");
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
    {NULL, NULL}
};

void shell_execute(const char* cmd) {
    if (cmd[0] == '\0') return;
    char command[INPUT_BUF_SIZE];
    const char *args;
    args = kstrchr(cmd, ' ');
    if (args) {
        size_t command_len = (size_t)(args - cmd);
        if (command_len >= INPUT_BUF_SIZE) command_len = INPUT_BUF_SIZE - 1;
        kstrncpy(command, cmd, command_len);
        command[command_len] = '\0';
        args++;
    } else {
        kstrncpy(command, cmd, INPUT_BUF_SIZE - 1);
        command[INPUT_BUF_SIZE - 1] = '\0';
        args = NULL;
    }
    int found = 0;
    for (int i = 0; commands[i].name != NULL; i++) {
        if (kstrcmp(command, commands[i].name) == 0) {
            commands[i].func(args);
            found = 1;
            break;
        }
    }
    if (!found) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print(cmd);
        print(": command not found\n");
        set_text_color(default_text_fg_color, default_text_bg_color);
    }
}

void shell_run() {
    char input[INPUT_BUF_SIZE] = {0};
    int idx = 0;
    int cursor_index = 0;
    print_prompt();
    prompt_start_vga_pos = get_cursor();
    while (1) {
        int c = keyboard_getchar();
        if (c == KEY_NULL) {
            continue;
        }
        if (c == KEY_LEFT) {
            if (cursor_index > 0) {
                cursor_index--;
                set_cursor_pos(prompt_start_vga_pos + cursor_index);
            }
            continue;
        }
        if (c == KEY_RIGHT) {
            if (cursor_index < idx) {
                cursor_index++;
                set_cursor_pos(prompt_start_vga_pos + cursor_index);
            }
            continue;
        }
        if (c == KEY_UP) {
            if (history_count == 0) continue;
            if (history_view_pos == -1) history_view_pos = history_count - 1;
            else if (history_view_pos > 0) history_view_pos--;
            load_history_line(input, &idx, &cursor_index, history_view_pos);
            continue;
        }
        if (c == KEY_DOWN) {
            if (history_count == 0) continue;
            if (history_view_pos == -1) continue;
            if (history_view_pos < history_count - 1) {
                history_view_pos++;
                load_history_line(input, &idx, &cursor_index, history_view_pos);
            } else {
                clear_input_line(get_screen_width());
                idx = 0;
                cursor_index = 0;
                input[0] = '\0';
                set_cursor_pos(prompt_start_vga_pos);
                history_view_pos = -1;
            }
            continue;
        }
        if (c == KEY_HOME) {
            cursor_index = 0;
            set_cursor_pos(prompt_start_vga_pos + cursor_index);
            continue;
        }
        if (c == KEY_END) {
            cursor_index = idx;
            set_cursor_pos(prompt_start_vga_pos + cursor_index);
            continue;
        }
        if (c == KEY_ENTER) {
            input[idx] = '\0';
            putchar('\n');
            add_history(input);
            shell_execute(input);
            if (get_cursor() % get_screen_width() != 0) {
                putchar('\n');
            }
            idx = 0;
            cursor_index = 0;
            input[0] = '\0';
            print_prompt();
            prompt_start_vga_pos = get_cursor();
            continue;
        }
        if (c == KEY_BACKSPACE) {
            if (cursor_index > 0) {
                for (int i = cursor_index - 1; i < idx; i++) {
                    input[i] = input[i + 1];
                }
                idx--;
                cursor_index--;
                input[idx] = '\0';
                clear_input_line(get_screen_width());
                set_cursor_pos(prompt_start_vga_pos);
                print(input);
                set_cursor_pos(prompt_start_vga_pos + cursor_index);
            }
            continue;
        }
        if (c == KEY_DELETE) {
            if (cursor_index < idx) {
                for (int i = cursor_index; i < idx; i++) {
                    input[i] = input[i + 1];
                }
                idx--;
                input[idx] = '\0';
                clear_input_line(get_screen_width());
                set_cursor_pos(prompt_start_vga_pos);
                print(input);
                set_cursor_pos(prompt_start_vga_pos + cursor_index);
            }
            continue;
        }
        if (idx < INPUT_BUF_SIZE - 1 && c >= 32 && c <= 126) {
            for (int i = idx; i > cursor_index; i--) input[i] = input[i - 1];
            input[cursor_index] = (char)c;
            idx++;
            cursor_index++;
            input[idx] = '\0';
            clear_input_line(get_screen_width());
            set_cursor_pos(prompt_start_vga_pos);
            print(input);
            set_cursor_pos(prompt_start_vga_pos + cursor_index);
        }
    }
}
