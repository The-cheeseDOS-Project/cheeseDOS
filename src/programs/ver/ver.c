#include "vga.h"
#include "version.h"
#include "programs.h"

void ver(const char*) {
    print("cheeseDOS version ");
    print(_binary_src_version_version_txt_start);
    print("\n");
}
