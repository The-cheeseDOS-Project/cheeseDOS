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

#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "elf.h"
#include "floppy.h"

bool load_elf(uint32_t start_lba, uint32_t load_addr) {
    Elf32_Ehdr elf_header;
    uint8_t sector_buf[512];

    if (!floppy_read(start_lba, 1, sector_buf)) return false;
    memcpy(&elf_header, sector_buf, sizeof(Elf32_Ehdr));

    if (elf_header.e_ident[0] != 0x7F || elf_header.e_ident[1] != 'E' ||
        elf_header.e_ident[2] != 'L' || elf_header.e_ident[3] != 'F') {
        return false;
    }

    uint32_t ph_offset = elf_header.e_phoff;

    for (uint16_t i = 0; i < elf_header.e_phnum; i++) {
        Elf32_Phdr phdr;
        uint32_t phdr_lba = start_lba + (ph_offset + i * sizeof(Elf32_Phdr)) / 512;
        uint32_t phdr_offset = (ph_offset + i * sizeof(Elf32_Phdr)) % 512;

        if (!floppy_read(phdr_lba, 1, sector_buf)) return false;
        memcpy(&phdr, sector_buf + phdr_offset, sizeof(Elf32_Phdr));

        if (phdr.p_type != 1) continue;

        uint32_t seg_lba = start_lba + phdr.p_offset / 512;
        uint32_t seg_count = (phdr.p_filesz + 511) / 512;
        void *dest = (void *)(load_addr + phdr.p_paddr);

        for (uint32_t j = 0; j < seg_count; j++) {
            if (!floppy_read(seg_lba + j, 1, sector_buf)) return false;
            uint32_t copy_size = (j == seg_count - 1)
                ? (phdr.p_filesz % 512 ? phdr.p_filesz % 512 : 512)
                : 512;
            memcpy((uint8_t *)dest + j * 512, sector_buf, copy_size);
        }

        if (phdr.p_memsz > phdr.p_filesz) {
            memset((uint8_t *)dest + phdr.p_filesz, 0,
                   phdr.p_memsz - phdr.p_filesz);
        }
    }

    void (*entry)(void) = (void (*)(void))(load_addr + elf_header.e_entry);
    entry();
    return true;
}
