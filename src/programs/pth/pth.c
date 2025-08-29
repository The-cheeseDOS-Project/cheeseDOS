#include "programs.h"
#include "vga.h"
#include "ramdisk.h"

void pth(const char*) {
    char path_buffer[256];
    if (ramdisk_get_path(current_dir_inode_no, path_buffer, sizeof(path_buffer)) == 0) {
        print(path_buffer);
    } else {
        print("Error getting path\n");
    }
}