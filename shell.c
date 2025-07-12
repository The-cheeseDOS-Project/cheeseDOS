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
 * but WITHOUT ANY WARRANTY; without even even implied warranty of
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
#include <stddef.h>
#include <stdint.h>
#include "rtc.h"

#define INPUT_BUF_SIZE 256
#define HISTORY_SIZE 32

static int prompt_start_vga_pos;
static char history[HISTORY_SIZE][INPUT_BUF_SIZE];
static int history_count = 0;
static int history_pos = 0;
static int history_view_pos = -1;
static uint32_t current_dir_inode_no = 0;

void putchar(char c);
int ramdisk_writefile(ramdisk_inode_t *file, uint32_t offset, uint32_t size, const char *buffer);

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
    ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
    if (dir) {
        print(dir->name);
        print("> ");
    } else {
        print("/> ");
    }
    prompt_start_vga_pos = get_cursor();
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
    set_cursor_pos(prompt_start_vga_pos);
    for (int i = 0; i < len; i++) putchar(' ');
    set_cursor_pos(prompt_start_vga_pos);
}

static void load_history_line(char *input, int *idx, int *cursor_index, int pos) {
    if (pos < 0 || pos >= history_count) return;
    clear_input_line(*idx);
    kstrncpy(input, history[pos], INPUT_BUF_SIZE - 1);
    input[INPUT_BUF_SIZE - 1] = '\0';
    *idx = kstrlen(input);
    *cursor_index = *idx;
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
    putchar(' '); // Added space here
    print(ampm);
    print("\n");
}


// command parsing

// command handlers

void shell_cmd_hlp(__attribute__((unused)) const char *args) {
	print("Commands: hlp, cls, say, ver, hi, ls, see, add, rem, mkd, cd, sum, rtc\n");
}

void shell_cmd_ver(__attribute__((unused)) const char *args) { print("cheeseDOS alpha\n"); }

void shell_cmd_hi(__attribute__((unused)) const char *args) { print("Hello, world!\n"); }

void shell_cmd_cls(__attribute__((unused)) const char *args) { clear_screen(); }

void shell_cmd_say(const char *args) {
	if (args) { print(args); }
	print("\n");
}

void shell_cmd_sum(const char *args) { calc_command(args ? args : ""); }

void shell_cmd_ls(__attribute__((unused)) const char *args) {
	ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
	if (!dir) {
		print("Failed to get directory inode\n");
		return;
	}
	ramdisk_readdir(dir, print_name_callback);
}

void shell_cmd_see(const char *args) {
	if (!args) {
		print("Usage: see <filename>\n");
		return;
	}
	const char *filename = args;
	ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
	if (!dir) {
		print("Failed to get current directory\n");
		return;
	}
	ramdisk_inode_t *file = ramdisk_find_inode_by_name(dir, filename);
	if (!file) {
		print("File not found\n");
		return;
	}
	if (file->type == RAMDISK_INODE_TYPE_DIR) {
		print("Cannot see directory\n");
		return;
	}
	char buf[256];
	int read = ramdisk_readfile(file, 0, sizeof(buf) - 1, buf);
	if (read < 0) {
		print("Error reading file\n");
		return;
	}
	buf[read] = 0;
	print(buf);
	print("\n");
}

void shell_cmd_add(const char *args) {
	if (!args) {
		print("Usage: add <filename> \"text to add\"\n");
		return;
	}
	const char *space_pos = kstrchr(args, ' ');
	if (!space_pos) {
		int res = ramdisk_create_file(current_dir_inode_no, args);
		if (res == 0) {
			print("File created\n");
		} else {
			print("Failed to create file\n");
		}
		return;
	}
	size_t filename_len = (size_t) (space_pos - args);
	char filename[INPUT_BUF_SIZE];
	if (filename_len >= INPUT_BUF_SIZE) filename_len = INPUT_BUF_SIZE - 1;
	kstrncpy(filename, args, filename_len);
	filename[filename_len] = '\0';

	const char *text_start = space_pos + 1;
	char text[INPUT_BUF_SIZE];
	size_t text_len = kstrlen(text_start);
	if (text_start[0] == '"' && text_start[text_len - 1] == '"') {
		kstrncpy(text, text_start + 1, text_len - 2);
		text[text_len - 2] = '\0';
	} else {
		kstrncpy(text, text_start, INPUT_BUF_SIZE - 1);
		text[INPUT_BUF_SIZE - 1] = '\0';
	}

	ramdisk_inode_t *dir = ramdisk_iget(current_dir_inode_no);
	if (!dir) {
		print("Failed to get current directory\n");
		return;
	}
	ramdisk_inode_t *file = ramdisk_find_inode_by_name(dir, filename);
	if (!file) {
		int res = ramdisk_create_file(current_dir_inode_no, filename);
		if (res != 0) {
			print("Failed to create file\n");
			return;
		}
		file = ramdisk_find_inode_by_name(dir, filename);
		if (!file) {
			print("File creation error\n");
			return;
		}
	}

	char old_content[1024];
	int old_len = ramdisk_readfile(file, 0, sizeof(old_content) - 1, old_content);
	if (old_len < 0) old_len = 0;
	old_content[old_len] = '\0';

	char new_content[2048];
	kstrncpy(new_content, old_content, sizeof(new_content) - 1);
	size_t new_len = kstrlen(new_content);
	if (new_len > 0 && new_content[new_len - 1] != '\n') {
		new_content[new_len] = '\n';
		new_content[new_len + 1] = '\0';
		new_len++;
	}

	size_t text_len2 = kstrlen(text);
	if (new_len + text_len2 + 1 < sizeof(new_content)) {
		kstrncpy(new_content + new_len, text, sizeof(new_content) - new_len - 1);
		new_len += text_len2;
		new_content[new_len] = '\0';
	} else {
		print("Text too long to append\n");
		return;
	}

	int write_res = ramdisk_writefile(file, 0, (uint32_t) new_len, new_content);
	if (write_res < 0) {
		print("Failed to write to file\n");
		return;
	}

	print("Text added\n");
}

void shell_cmd_rem(const char *args) {
	if (!args) {
		print("Usage: rem <filename>\n");
		return;
	}
	int res = ramdisk_remove_file(current_dir_inode_no, args);
	if (res == 0) {
		print("File removed\n");
	} else {
		print("Failed to remove file\n");
	}
}

void shell_cmd_mkd(const char *args) {
	if (!args) {
		print("Usage: mkd <dirname>\n");
		return;
	}
	int res = ramdisk_create_dir(current_dir_inode_no, args);
	if (res == 0) {
		print("Directory created\n");
	} else {
		print("Failed to create directory\n");
	}
}

void shell_cmd_cd(const char *args) {
	if (!args) {
		print("Usage: cd <dirname>\n");
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
		print("Failed to get current directory\n");
		return;
	}
	ramdisk_inode_t *new_dir = ramdisk_find_inode_by_name(dir, dirname);
	if (!new_dir || new_dir->type != RAMDISK_INODE_TYPE_DIR) {
		print("Directory not found\n");
		return;
	}
	current_dir_inode_no = new_dir->inode_no;
}

void shell_cmd_rtc(__attribute__((unused)) const char *args) { handle_rtc_command(); }

// !command handlers

// define the structure that holds the command information
typedef struct {
	const char *cmd;				  // the command's name (eg. "cls")
	uint64_t hash;					  //the command's hash as computed by the djb2 algorithm
	void (*handler)(const char *args);// the function pointer to the command's handler
} cmd_t;

cmd_t commands[MAX_CMD_COUNT];// an array holding command information structures

// simple function to hash C strings using the djb2 algorithm
static uint64_t djb2_hash(const char *str) {
	uint64_t hash = 5381;
	int c;
	while ((c = *str++)) { hash = ((hash << 5) + hash) + c; }
	return hash;
}

#define DEFINE_CMD(name)                                                                                               \
	(cmd_t) { #name, djb2_hash(#name), shell_cmd_##name }

// helper function to register commands (registers the command name, it's hash, and a pointer to the handler function)
void register_commands() {
	commands[0] = DEFINE_CMD(hlp);
	commands[1] = DEFINE_CMD(ver);
	commands[2] = DEFINE_CMD(hi);
	commands[3] = DEFINE_CMD(cls);
	commands[4] = DEFINE_CMD(say);
	commands[5] = DEFINE_CMD(sum);
	commands[6] = DEFINE_CMD(see);
	commands[7] = DEFINE_CMD(add);
	commands[8] = DEFINE_CMD(rem);
	commands[9] = DEFINE_CMD(mkd);
	commands[10] = DEFINE_CMD(cd);
	commands[11] = DEFINE_CMD(rtc);
	commands[12] = DEFINE_CMD(ls);
	// if more than 16 commands are added, MAX_CMD_COUNT has to be updated. (alternatively dynamically allocate `cmd_t commands` array)
}

// !command parsing

void shell_execute(const char *cmd) {
	if (cmd[0] == '\0') return;

	char command[INPUT_BUF_SIZE];
	const char *args;

	args = kstrchr(cmd, ' ');
	if (args) {
		size_t command_len = (size_t) (args - cmd);
		if (command_len >= INPUT_BUF_SIZE) command_len = INPUT_BUF_SIZE - 1;
		kstrncpy(command, cmd, command_len);
		command[command_len] = '\0';
		args++;
	} else {
		kstrncpy(command, cmd, INPUT_BUF_SIZE - 1);
		command[INPUT_BUF_SIZE - 1] = '\0';
		args = NULL;
	}

	uint64_t cmdHash = djb2_hash(command);

	// itterate thought the list of commands
	for (size_t i = 0; i < MAX_CMD_COUNT; i++) {
        // compare the cached hash of the command
		if (commands[i].hash == cmdHash) {
            // if the hash matches, verify match by comparing the cached literal of the command
			if (kstrcmp(commands[i].cmd, command) == 0) {
                // if both the hash and the literal match, execute the handler
				commands[i].handler(args);
				return;
			}
		}
	}

	print(cmd);
	print(": command not found\n");
}

void shell_run() {
    char input[INPUT_BUF_SIZE] = {0};
    int idx = 0;
    int cursor_index = 0;

    print_prompt();

    while (1) {
        int c = keyboard_getchar();

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
                clear_input_line(idx);
                idx = 0;
                cursor_index = 0;
                input[0] = '\0';
                set_cursor_pos(prompt_start_vga_pos);
                history_view_pos = -1;
            }
            continue;
        }
        if (c == '\n') {
            input[idx] = '\0';
            putchar('\n');
            add_history(input);
            shell_execute(input);
            idx = 0;
            cursor_index = 0;
            input[0] = '\0';
            print_prompt();
            continue;
        }
        if (c == '\b') {
            if (cursor_index > 0) {
                for (int i = cursor_index - 1; i < idx - 1; i++) input[i] = input[i + 1];
                idx--;
                cursor_index--;
                set_cursor_pos(prompt_start_vga_pos + cursor_index);
                for (int i = cursor_index; i < idx; i++) putchar(input[i]);
                putchar(' ');
                set_cursor_pos(prompt_start_vga_pos + cursor_index);
            }
            continue;
        }
        if (idx < INPUT_BUF_SIZE - 1 && c >= 32 && c <= 126) {
            for (int i = idx; i > cursor_index; i--) input[i] = input[i - 1];
            input[cursor_index] = c;
            idx++;
            cursor_index++;
            set_cursor_pos(prompt_start_vga_pos);
            for (int i = 0; i < idx; i++) putchar(input[i]);
            set_cursor_pos(prompt_start_vga_pos + cursor_index);
        }
    }
}