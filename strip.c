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
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>

#define TARGET_PATH "build/cheesedos.elf"

int main() {
    int fd = open(TARGET_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    uint8_t *data = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)data;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0 || ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
        fprintf(stderr, "Not a valid ELF32 file\n");
        munmap(data, st.st_size);
        close(fd);
        return 1;
    }

    Elf32_Phdr *phdr = (Elf32_Phdr *)(data + ehdr->e_phoff);
    Elf32_Off max_offset = 0;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            Elf32_Off end = phdr[i].p_offset + phdr[i].p_filesz;
            if (end > max_offset) max_offset = end;
        }
    }

    ehdr->e_shoff = 0;
    ehdr->e_shnum = 0;
    ehdr->e_shstrndx = SHN_UNDEF;

    if (ftruncate(fd, max_offset) < 0) {
        perror("ftruncate");
        munmap(data, st.st_size);
        close(fd);
        return 1;
    }

    munmap(data, st.st_size);
    close(fd);

    return 0;
}
