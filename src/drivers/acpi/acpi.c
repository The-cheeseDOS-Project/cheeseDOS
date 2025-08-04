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
 
#include <stdint.h>
#include <stddef.h>

#define SLP_TYP (5 << 10)  
#define SLP_EN  (1 << 13)  

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__("outw %0, %1" : : "a"(val), "Nd"(port));
}

static uint16_t pm1a_cnt = 0;  

struct RSDPDescriptor {
    char     Signature[8];
    uint8_t  Checksum;
    char     OEMID[6];
    uint8_t  Revision;
    uint32_t RsdtAddress;
};

struct ACPISDTHeader {
    char     Signature[4];
    uint32_t Length;
    uint8_t  Revision;
    uint8_t  Checksum;
    char     OEMID[6];
    char     OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
};

struct FADT {
    struct ACPISDTHeader h;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;
    uint8_t  Reserved;
    uint8_t  PreferredPMProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  ACPI_Enable;
    uint8_t  ACPI_Disable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1a_EVT_BLK;
    uint32_t PM1b_EVT_BLK;
    uint32_t PM1a_CNT_BLK;

};

int acpi_checksum(uint8_t* ptr, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++)
        sum += ptr[i];
    return sum == 0;
}

int find_acpi() {
    for (uint32_t addr = 0x000E0000; addr < 0x00100000; addr += 16) {
        struct RSDPDescriptor* rsdp = (struct RSDPDescriptor*)addr;
        if (!__builtin_memcmp(rsdp->Signature, "RSD PTR ", 8) &&
            acpi_checksum((uint8_t*)rsdp, sizeof(struct RSDPDescriptor))) {

            struct ACPISDTHeader* rsdt = (struct ACPISDTHeader*)(uintptr_t)rsdp->RsdtAddress;
            if (!__builtin_memcmp(rsdt->Signature, "RSDT", 4)) {
                int entryCount = (rsdt->Length - sizeof(struct ACPISDTHeader)) / 4;
                uint32_t* entries = (uint32_t*)((uintptr_t)rsdt + sizeof(struct ACPISDTHeader));
                for (int i = 0; i < entryCount; i++) {
                    struct ACPISDTHeader* hdr = (struct ACPISDTHeader*)(uintptr_t)entries[i];
                    if (!__builtin_memcmp(hdr->Signature, "FACP", 4)) {
                        struct FADT* fadt = (struct FADT*)hdr;
                        pm1a_cnt = (uint16_t)fadt->PM1a_CNT_BLK;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void shutdown() {

    uint16_t ports[] = {
        0xB004, 
        0x604,  
        0x4004, 
        0x600   
    };

    uint16_t values[] = {
        0x2000, 
        0x3400, 
        0x34    
    };

    for (int i = 0; i < sizeof(ports)/sizeof(ports[0]); i++) {
        for (int j = 0; j < sizeof(values)/sizeof(values[0]); j++) {
            outw(ports[i], values[j]);
        }
    }

    if (pm1a_cnt) {
        outw(pm1a_cnt, SLP_TYP | SLP_EN);
    }
}

void reboot() {
    outw(0x64, 0xFE);  
}