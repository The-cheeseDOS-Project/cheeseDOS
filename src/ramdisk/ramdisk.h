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

#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdint.h>
#include <stddef.h>

#define RAMDISK_FILENAME_MAX 32
#define RAMDISK_DATA_SIZE_BYTES 48

typedef enum {
    RAMDISK_INODE_TYPE_UNUSED = 0,
    RAMDISK_INODE_TYPE_FILE = 1,
    RAMDISK_INODE_TYPE_DIR = 2,
} ramdisk_inode_type_t;

typedef struct {
    uint32_t inode_no;
    ramdisk_inode_type_t type;
    uint32_t size;
    uint32_t parent_inode_no;
    char name[RAMDISK_FILENAME_MAX];
    uint8_t data[RAMDISK_DATA_SIZE_BYTES];
} ramdisk_inode_t;

void ramdisk_init();

ramdisk_inode_t* ramdisk_iget(uint32_t inode_no);

typedef void (*ramdisk_readdir_callback)(const char *name, uint32_t inode_no);
void ramdisk_readdir(ramdisk_inode_t *dir, ramdisk_readdir_callback cb);

int ramdisk_create_file(uint32_t parent_dir_inode_no, const char *filename);

int ramdisk_create_dir(uint32_t parent_dir_inode_no, const char *dirname);

int ramdisk_remove_file(uint32_t parent_dir_inode_no, const char *filename);

int ramdisk_readfile(ramdisk_inode_t *file, uint32_t offset, uint32_t size, char *buffer);

int ramdisk_writefile(ramdisk_inode_t *file, uint32_t offset, uint32_t len, const char *buffer);

int ramdisk_get_path(uint32_t inode_no, char *buffer, size_t buffer_size);

#endif