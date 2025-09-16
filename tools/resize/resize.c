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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define CONFIG_FILE "config.conf"
#define KEY_FLOPPY "FLOPPY"
#define FLOPPY_SIZE 1474560

int truncate(const char *path, off_t length);

void get_floppy_filename(char *filename, size_t size) {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, KEY_FLOPPY, strlen(KEY_FLOPPY)) == 0) {
            char *value = strchr(line, '=');
            if (value != NULL) {
                value++;
                size_t len = strlen(value);
                if (len > 0 && value[len - 1] == '\n') {
                    value[len - 1] = '\0';
                }
                snprintf(filename, size, "%s", value);
                fclose(file);
                return;
            }
        }
    }

    fclose(file);
    filename[0] = '\0';
}

int main() {
    char floppy_path[256];
    get_floppy_filename(floppy_path, sizeof(floppy_path));

    if (floppy_path[0] == '\0') {
        return 1;
    }

    if (truncate(floppy_path, FLOPPY_SIZE) == 0) {
        return 0;
    } else {
        return 1;
    }
}
