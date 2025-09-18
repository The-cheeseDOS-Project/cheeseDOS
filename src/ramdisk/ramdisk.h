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

#define RAMDISK_FILENAME_MAX 32
#define RAMDISK_DATA_SIZE_BYTES 512

#define ROOT_INODE 0

enum ramdisk_inode_type {
    RAMDISK_INODE_TYPE_UNUSED = 0,
    RAMDISK_INODE_TYPE_FILE = 1,
    RAMDISK_INODE_TYPE_DIR = 2,
};

typedef struct ramdisk_inode ramdisk_inode_t;
typedef void (*ramdisk_readdir_callback)(const char *name, unsigned int inode_no);

struct ramdisk_inode {
    unsigned int inode_no;
    enum ramdisk_inode_type type;
    unsigned int size;
    unsigned int parent_inode_no;
    char name[RAMDISK_FILENAME_MAX];
    unsigned char data[RAMDISK_DATA_SIZE_BYTES];
};

extern unsigned int current_dir_inode_no;

void ramdisk_init();
struct ramdisk_inode* ramdisk_iget(unsigned int inode_no);
void ramdisk_readdir(struct ramdisk_inode *dir, void (*cb)(const char *name, unsigned int inode_no));
int ramdisk_create_file(unsigned int parent_dir_inode_no, const char *filename);
int ramdisk_create_dir(unsigned int parent_dir_inode_no, const char *dirname);
int ramdisk_remove_file(unsigned int parent_dir_inode_no, const char *filename);
int ramdisk_readfile(struct ramdisk_inode *file, unsigned int offset, unsigned int size, char *buffer);
int ramdisk_writefile(struct ramdisk_inode *file, unsigned int offset, unsigned int len, const char *buffer);
int ramdisk_get_path(unsigned int inode_no, char *buffer, unsigned int buffer_size);
struct ramdisk_inode* ramdisk_iget_by_name(unsigned int parent_inode, const char *name);
struct ramdisk_inode* ramdisk_find_inode_by_name(struct ramdisk_inode *dir, const char *name);
void copy_inode(struct ramdisk_inode *src, struct ramdisk_inode *dst);
int ramdisk_find_inode_by_path(const char *path, unsigned int *inode_no);

#endif
