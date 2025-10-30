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

#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include "beep.h"
#include "timer.h"
#include "serial.h"
#include "ramdisk.h"
#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"

#define INPUT_BUF_SIZE 256
#define HISTORY_SIZE 32

static int prompt_start_vga_pos;
static char history[HISTORY_SIZE][INPUT_BUF_SIZE];
static int history_count = 0;
static int history_pos = 0;
static int history_view_pos = -1;

extern void print(const char* str);
extern void print_ansi(const char* ansi_str);

static void print_prompt() {
    set_text_color(COLOR_CYAN, COLOR_BLACK);
    shell_execute("pth"); // Yes I know it's a bit hacky but hey, it works!
    set_text_color(COLOR_YELLOW, COLOR_BLACK);
    print("> ");
    set_text_color(COLOR_WHITE, COLOR_BLACK);
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

static void trim_whitespace(char *str) {
    if (!str) return;

    int start = 0;
    while (str[start] == ' ' || str[start] == '\t') start++;

    if (start > 0) {
        int i;
        for (i = 0; str[start + i]; i++) {
            str[i] = str[start + i];
        }
        str[i] = '\0';
    }

    int end = kstrlen(str) - 1;
    while (end >= 0 && (str[end] == ' ' || str[end] == '\t')) {
        str[end] = '\0';
        end--;
    }
}

static bool execute_single_command(const char* cmd) {
    char command[INPUT_BUF_SIZE];
    const char *args;

    kstrncpy(command, cmd, INPUT_BUF_SIZE - 1);
    command[INPUT_BUF_SIZE - 1] = '\0';

    trim_whitespace(command);

    if (command[0] == '\0') return true;

    args = kstrchr(command, ' ');
    if (args) {
        size_t command_len = (size_t)(args - command);
        char temp_cmd[INPUT_BUF_SIZE];
        kstrncpy(temp_cmd, command, command_len);
        temp_cmd[command_len] = '\0';

        kstrcpy(command, temp_cmd);
        args++;

        while (*args == ' ') args++;
        if (*args == '\0') args = NULL;
    }

    sprint("Running: ");
    sprint(command);
    if (args) {
        sprint(" ");
        sprint(args);
    }
    sprint("...");

    bool success = execute_command(command, args);
    if (success) {
        sprint(" \033[92mDone!\033[0m\n");
    }

    return success;
}

void shell_execute(const char* cmd) {
    if (cmd[0] == '\0') return;

    const char *amp_pos = kstrchr(cmd, '&');

    if (amp_pos == NULL) {
        execute_single_command(cmd);
        return;
    }

    char cmd_copy[INPUT_BUF_SIZE];
    kstrncpy(cmd_copy, cmd, INPUT_BUF_SIZE - 1);
    cmd_copy[INPUT_BUF_SIZE - 1] = '\0';

    char *current_cmd = cmd_copy;
    char *next_amp;

    while (current_cmd != NULL) {
        next_amp = (char*)kstrchr(current_cmd, '&');

        if (next_amp != NULL) {
            *next_amp = '\0';
        }

        execute_single_command(current_cmd);

        if (next_amp != NULL) {
            current_cmd = next_amp + 1;
        } else {
            current_cmd = NULL;
        }
    }
}

void shell_run() {
    char input[INPUT_BUF_SIZE];
    int idx = 0;
    int cursor_index = 0;

    set_text_color(COLOR_GREEN, COLOR_BLACK);
    print(" Done!\n");
    set_text_color(COLOR_WHITE, COLOR_BLACK);

    sprint(" \033[92mDone!\033[0m\n");

    print("\ncheeseDOS is free software: you may redistribute and/or modify it under the \nterms of the GNU GPLv3. It comes with ABSOLUTELY NO WARRANTY, to the extent \npermitted by law.\n\n");

    print_prompt();
    prompt_start_vga_pos = get_cursor();

    beep(420, 100);
    beep(480, 100);
    beep(420, 200);

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
            vga_putchar('\n');
            add_history(input);
            shell_execute(input);
            if (get_cursor() % get_screen_width() != 0) {
                vga_putchar('\n');
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
