/*
 * cheeseDOS - My x86 DOS
 * Copyright (C) 2025 Kross1de
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

#ifndef VFS_H
#define VFS_H

#include "stdint.h"
#include "stddef.h"

#define VFS_PATH_MAX 256
#define VFS_FILENAME_MAX 32

typedef enum {
    VFS_TYPE_UNUSED = 0,
    VFS_TYPE_FILE = 1,
    VFS_TYPE_DIR = 2,
} vfs_type_t;

typedef struct {
    uint32_t inode_no;
    vfs_type_t type;
    uint32_t size;
    char name[VFS_FILENAME_MAX];
    // i will add more i think but not now
} vfs_node_t;

typedef void (*vfs_readdir_callback)(const char *name, uint32_t inode_no);

struct vfs_operations {
    vfs_node_t* (*iget)(uint32_t inode_no);
    int (*create_file)(uint32_t parent_inode, const char *name);
    int (*create_dir)(uint32_t parent_inode, const char *name);
    int (*remove)(uint32_t parent_inode, const char *name);
    int (*read)(vfs_node_t *node, uint32_t offset, uint32_t size, char *buffer);
    int (*write)(vfs_node_t *node, uint32_t offset, uint32_t size, const char *buffer);
    void (*readdir)(vfs_node_t *dir, vfs_readdir_callback cb);
    vfs_node_t* (*iget_by_name)(uint32_t parent_inode, const char *name);
    int (*get_path)(uint32_t inode_no, char *buffer, size_t buffer_size);
    // i will add more i think but not now
};

typedef struct vfs_mount {
    char mount_point[VFS_PATH_MAX];
    struct vfs_operations *ops;
    void *fs_data;
} vfs_mount_t;

void vfs_init();
vfs_node_t* vfs_resolve_path(const char *path, uint32_t *parent_inode_out);
int vfs_create_file(const char *path);
int vfs_create_dir(const char *path);
int vfs_remove(const char *path);
int vfs_read(const char *path, uint32_t offset, uint32_t size, char *buffer);
int vfs_write(const char *path, uint32_t offset, uint32_t size, const char *buffer);
void vfs_readdir(const char *path, vfs_readdir_callback cb);
int vfs_get_path(const char *rel_path, char *buffer, size_t buffer_size);
vfs_node_t* vfs_iget_by_name(const char *path);

extern char current_dir_path[VFS_PATH_MAX];

#endif
