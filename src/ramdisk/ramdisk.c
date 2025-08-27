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

#include "ramdisk.h" 
#include <stdint.h>

static ramdisk_inode_t inodes[32];


void ramdisk_init() {
    for (int i = 0; i < 32; i++) {
        inodes[i].inode_no = i;
        inodes[i].type = RAMDISK_INODE_TYPE_UNUSED;
        inodes[i].size = 0;
        inodes[i].parent_inode_no = 0;
        for (size_t j = 0; j < RAMDISK_FILENAME_MAX; j++) { 
            inodes[i].name[j] = 0;
        }
        for (size_t j = 0; j < sizeof(inodes[i].data); j++) { 
            inodes[i].data[j] = 0;
        }
    }
    inodes[0].type = RAMDISK_INODE_TYPE_DIR;
    inodes[0].inode_no = 0;
    inodes[0].parent_inode_no = 0;
    const char root_name[] = "/";
    for (size_t i = 0; i < sizeof(root_name) && i < RAMDISK_FILENAME_MAX; i++) inodes[0].name[i] = root_name[i];
}

ramdisk_inode_t* ramdisk_iget(uint32_t inode_no) {
    if (inode_no >= 32) return NULL;
    if (inodes[inode_no].type == RAMDISK_INODE_TYPE_UNUSED) return NULL;
    return &inodes[inode_no];
}

static int rm_create_entity(uint32_t parent_dir_inode_no, const char *name, ramdisk_inode_type_t type) {
    if (!name) return -1;
    if (kstrlen(name) >= RAMDISK_FILENAME_MAX) return -1;

    for (int i = 0; i < 32; i++) {
        if (inodes[i].type != RAMDISK_INODE_TYPE_UNUSED) {
            if (kstrcmp(inodes[i].name, name) == 0 && inodes[i].parent_inode_no == parent_dir_inode_no) return -1;
        }
    }
    for (int i = 0; i < 32; i++) {
        if (inodes[i].type == RAMDISK_INODE_TYPE_UNUSED) {
            inodes[i].type = type;
            inodes[i].parent_inode_no = parent_dir_inode_no;
            size_t len = kstrlen(name);
            memcpy(inodes[i].name, name, len);
            inodes[i].name[len] = 0;
            inodes[i].size = 0;
            return 0;
        }
    }
    return -1;

}

int ramdisk_create_file(uint32_t parent_dir_inode_no, const char *filename) {
  return rm_create_entity(parent_dir_inode_no, filename, RAMDISK_INODE_TYPE_FILE);
}

int ramdisk_create_dir(uint32_t parent_dir_inode_no, const char *dirname) {
  return rm_create_entity(parent_dir_inode_no, dirname, RAMDISK_INODE_TYPE_DIR);
}




int ramdisk_remove_file(uint32_t parent_dir_inode_no, const char *filename) {
    for (int i = 0; i < 32; i++) {
        if (inodes[i].type != RAMDISK_INODE_TYPE_UNUSED && inodes[i].parent_inode_no == parent_dir_inode_no) {
            if (kstrcmp(inodes[i].name, filename) == 0) {
                if (inodes[i].type == RAMDISK_INODE_TYPE_DIR) {
                    int is_empty = 1;
                    for (int k = 0; k < 32; k++) {
                        if (inodes[k].type != RAMDISK_INODE_TYPE_UNUSED && inodes[k].parent_inode_no == inodes[i].inode_no) {
                            is_empty = 0;
                            break;
                        }
                    }
                    if (!is_empty) {
                        return -1;
                    }
                }
                inodes[i].type = RAMDISK_INODE_TYPE_UNUSED;
                inodes[i].size = 0;
                inodes[i].parent_inode_no = 0;
                for (size_t j = 0; j < RAMDISK_FILENAME_MAX; j++) inodes[i].name[j] = 0; 
                for (size_t j = 0; j < sizeof(inodes[i].data); j++) inodes[i].data[j] = 0; 
                return 0;
            }
        }
    }
    return -1;
}

int ramdisk_readfile(ramdisk_inode_t *file, uint32_t offset, uint32_t size, char *buffer) {
    if (!file || !buffer) return -1;
    if (file->type != RAMDISK_INODE_TYPE_FILE) return -1;

    if (offset > file->size) return 0;
    if (offset + size > file->size) size = file->size - offset;

    memcpy(buffer, (const char*)&file->data[offset], size);
    return size;
}

int ramdisk_writefile(ramdisk_inode_t *file, uint32_t offset, uint32_t size, const char *buffer) {
    if (!file || !buffer) return -1;
    if (file->type != RAMDISK_INODE_TYPE_FILE) return -1;

    if (offset > sizeof(file->data)) return -1;
    if (offset + size > sizeof(file->data)) {
        size = sizeof(file->data) - offset;
    }

    memcpy(&file->data[offset], buffer, size);

    if (offset + size > file->size) {
        file->size = offset + size;
    }

    return size;
}

void ramdisk_readdir(ramdisk_inode_t *dir, ramdisk_readdir_callback cb) {
    if (!dir || dir->type != RAMDISK_INODE_TYPE_DIR || !cb) return;
    for (int i = 0; i < 32; i++) {
        if (inodes[i].type != RAMDISK_INODE_TYPE_UNUSED && inodes[i].parent_inode_no == dir->inode_no) {
            cb(inodes[i].name, inodes[i].inode_no);
        }
    }
}

ramdisk_inode_t* ramdisk_iget_by_name(uint32_t parent_inode, const char *name) {
    for (int i = 0; i < 32; i++) {
        if (inodes[i].type != RAMDISK_INODE_TYPE_UNUSED &&
            inodes[i].parent_inode_no == parent_inode &&
            kstrcmp(inodes[i].name, name) == 0) {
            return &inodes[i];
        }
    }
    return NULL;
}

int ramdisk_get_path(uint32_t inode_no, char *buffer, size_t buffer_size) {
    if (buffer_size == 0) {
        return -1;
    }

    uint32_t segments[32];
    int segment_count = 0;

    uint32_t current = inode_no;
    while (current != 0) {
        ramdisk_inode_t *node = ramdisk_iget(current);
        if (!node) {
            return -1;
        }
        if (segment_count >= 32) return -1;
        segments[segment_count++] = current;
        current = node->parent_inode_no;
    }

    if (inode_no == 0) {
        if (buffer_size < 2) return -1;
        kstrcpy(buffer, "/");
        return 0;
    }

    size_t final_path_len = 0;
    if (buffer_size < 1) return -1;
    buffer[0] = '\0';

    for (int i = segment_count - 1; i >= 0; i--) {
        ramdisk_inode_t *node = ramdisk_iget(segments[i]);
        if (!node) return -1;

        size_t name_len = kstrlen(node->name);
        if (final_path_len + 1 + name_len + 1 > buffer_size) {
            return -1;
        }

        kstrcpy(buffer + final_path_len, "/");
        final_path_len += 1;
        kstrcpy(buffer + final_path_len, node->name);
        final_path_len += name_len;
    }

    if (final_path_len == 0 && buffer_size >= 2) {
        kstrcpy(buffer, "/");
    } else if (final_path_len == 0 && buffer_size < 2) {
         return -1;
    }

    return 0;
}
