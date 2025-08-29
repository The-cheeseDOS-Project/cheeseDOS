#include "pth.h"

void pth(const char *args, uint32_t *cwd) {
    char path_buffer[256];
    if (ramdisk_get_path(*cwd, path_buffer, sizeof(path_buffer)) == 0) {
        print(path_buffer);
    } else {
        print("Error getting path\n");
    }
}
