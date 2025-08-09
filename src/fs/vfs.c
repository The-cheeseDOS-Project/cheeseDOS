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

#include "vfs.h"
#include "ramdisk.h"
#include "string.h"
#include "stddef.h"
#include "stdint.h"

static vfs_mount_t root_mount;
char current_dir_path[VFS_PATH_MAX] = "/";

extern struct vfs_operations ramdisk_vfs_ops;

static bool is_absolute_path(const char *path) {
    return path[0] == '/';
}

static void normalize_path(const char *path, char *out_path) {
    // TODO: implement this
}

vfs_node_t* vfs_resolve_path(const char *path, uint32_t *parent_inode_out) {
    // TODO: implement this
}

void vfs_init() {
    // TODO: implement this
}

int vfs_create_file(const char *path) {
    // TODO: implement this
}

int vfs_create_dir(const char *path) {
    // TODO: implement this
}

int vfs_remove(const char *path) {
    // TODO: implement this
}

int vfs_read(const char *path, uint32_t offset, uint32_t size, char *buffer) {
    // TODO: implement this
}

int vfs_write(const char *path, uint32_t offset, uint32_t size, const char *buffer) {
    // TODO: implement
}

void vfs_readdir(const char *path, vfs_readdir_callback cb) {
    // TODO: implement this
}

int vfs_get_path(const char *rel_path, char *buffer, size_t buffer_size) {
    // TODO: implement this
}

vfs_node_t* vfs_iget_by_name(const char *path) {
    // TODO: implement this
}
