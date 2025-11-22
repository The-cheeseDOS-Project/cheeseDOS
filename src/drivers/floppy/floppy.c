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

#include "stdint.h"
#include "stdbool.h"

#define FLOPPY_DOR  0x3F2  
#define FLOPPY_MSR  0x3F4  
#define FLOPPY_FIFO 0x3F5  
#define FLOPPY_CCR  0x3F7  

#define CMD_READ_DATA    0xE6
#define CMD_SEEK         0x0F
#define CMD_SENSE_INT    0x08
#define CMD_SPECIFY      0x03
#define CMD_RECALIBRATE  0x07

static uint8_t dma_buffer[512] __attribute__((aligned(512)));
static volatile bool irq_received = false;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static bool floppy_wait_ready(void) {
    for (int i = 0; i < 600; i++) {
        uint8_t msr = inb(FLOPPY_MSR);
        if (msr & 0x80) return true;

        for (volatile int j = 0; j < 1000; j++);
    }
    return false;
}

static bool floppy_send_byte(uint8_t byte) {
    if (!floppy_wait_ready()) return false;
    outb(FLOPPY_FIFO, byte);
    return true;
}

static bool floppy_read_byte(uint8_t *byte) {
    if (!floppy_wait_ready()) return false;
    *byte = inb(FLOPPY_FIFO);
    return true;
}

static void floppy_setup_dma(uint32_t addr, uint16_t count) {
    outb(0x0A, 0x06);  
    outb(0x0C, 0xFF);  
    outb(0x04, addr & 0xFF);  
    outb(0x04, (addr >> 8) & 0xFF);  
    outb(0x81, (addr >> 16) & 0xFF);  
    outb(0x0C, 0xFF);  
    outb(0x05, (count - 1) & 0xFF);  
    outb(0x05, ((count - 1) >> 8) & 0xFF);  
    outb(0x0B, 0x56);  
    outb(0x0A, 0x02);  
}

void floppy_irq_handler(void) {
    irq_received = true;
}

static bool floppy_wait_irq(void) {
    for (int i = 0; i < 1000; i++) {
        if (irq_received) {
            irq_received = false;
            return true;
        }
        for (volatile int j = 0; j < 10000; j++);
    }
    return false;
}

static bool floppy_sense_interrupt(uint8_t *st0, uint8_t *cyl) {
    if (!floppy_send_byte(CMD_SENSE_INT)) return false;
    if (!floppy_read_byte(st0)) return false;
    if (!floppy_read_byte(cyl)) return false;
    return true;
}

static bool floppy_reset(void) {
    outb(FLOPPY_DOR, 0x00);  
    outb(FLOPPY_DOR, 0x0C);  

    if (!floppy_wait_irq()) return false;

    uint8_t st0, cyl;
    for (int i = 0; i < 4; i++) {
        floppy_sense_interrupt(&st0, &cyl);
    }

    outb(FLOPPY_CCR, 0x00);  

    if (!floppy_send_byte(CMD_SPECIFY)) return false;
    if (!floppy_send_byte(0xDF)) return false;  
    if (!floppy_send_byte(0x02)) return false;  

    return true;
}

static bool floppy_seek(uint8_t cylinder) {
    if (!floppy_send_byte(CMD_SEEK)) return false;
    if (!floppy_send_byte(0x00)) return false;  
    if (!floppy_send_byte(cylinder)) return false;

    if (!floppy_wait_irq()) return false;

    uint8_t st0, cyl;
    return floppy_sense_interrupt(&st0, &cyl);
}

static bool floppy_read_sector(uint8_t cylinder, uint8_t head, uint8_t sector) {
    uint32_t phys_addr = (uint32_t)dma_buffer;
    floppy_setup_dma(phys_addr, 512);

    if (!floppy_send_byte(CMD_READ_DATA)) return false;
    if (!floppy_send_byte((head << 2) | 0)) return false;  
    if (!floppy_send_byte(cylinder)) return false;
    if (!floppy_send_byte(head)) return false;
    if (!floppy_send_byte(sector)) return false;
    if (!floppy_send_byte(2)) return false;  
    if (!floppy_send_byte(18)) return false;  
    if (!floppy_send_byte(0x1B)) return false;  
    if (!floppy_send_byte(0xFF)) return false;  

    if (!floppy_wait_irq()) return false;

    uint8_t st[7];
    for (int i = 0; i < 7; i++) {
        if (!floppy_read_byte(&st[i])) return false;
    }

    return (st[0] & 0xC0) == 0;
}

static void lba_to_chs(uint32_t lba, uint8_t *cyl, uint8_t *head, uint8_t *sector) {
    *sector = (lba % 18) + 1;
    *head = (lba / 18) % 2;
    *cyl = lba / 36;
}

int floppy_read(uint32_t lba, uint32_t count, void *buffer) {
    uint8_t *buf = (uint8_t *)buffer;

    for (uint32_t i = 0; i < count; i++) {
        uint8_t cyl, head, sector;
        lba_to_chs(lba + i, &cyl, &head, &sector);

        if (!floppy_seek(cyl)) return 0;
        if (!floppy_read_sector(cyl, head, sector)) return 0;

        for (int j = 0; j < 512; j++) {
            buf[i * 512 + j] = dma_buffer[j];
        }
    }

    return 1;
}

int floppy_init(void) {
    return floppy_reset() ? 1 : 0;
}
