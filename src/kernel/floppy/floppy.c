#include "floppy.h"
#include "stdint.h"
#include "io.h"

static uint8_t dma_buffer[BYTES_PER_SECTOR * 18] __attribute__((aligned(65536)));

static int floppy_wait_ready(void) {
    for (int i = 0; i < 600; i++) {
        uint8_t msr = inb(FLOPPY_MSR);
        if (msr & MSR_RQM) return 1;
        for (volatile int j = 0; j < 1000; j++);
    }
    return 0;
}

static int floppy_send_byte(unsigned char byte) {
    if (!floppy_wait_ready()) return 0;
    outb(FLOPPY_FIFO, byte);
    return 1;
}

static int floppy_read_byte(unsigned char *byte) {
    for (int i = 0; i < 600; i++) {
        uint8_t msr = inb(FLOPPY_MSR);
        if ((msr & MSR_RQM) && (msr & MSR_DIO)) {
            *byte = inb(FLOPPY_FIFO);
            return 1;
        }
        for (volatile int j = 0; j < 1000; j++);
    }
    return 0;
}

static void floppy_setup_dma(int write, unsigned long length) {
    uint32_t addr = (uint32_t)dma_buffer;
    outb(0x0A, 0x06);
    outb(0x0C, 0xFF);
    outb(0x04, addr & 0xFF);
    outb(0x04, (addr >> 8) & 0xFF);
    outb(0x81, (addr >> 16) & 0xFF);
    outb(0x0C, 0xFF);
    outb(0x05, (length - 1) & 0xFF);
    outb(0x05, ((length - 1) >> 8) & 0xFF);
    outb(0x0B, write ? 0x4A : 0x46);
    outb(0x0A, 0x02);
}

static int floppy_wait_irq(void) {
    for (volatile int i = 0; i < 100000; i++);
    return 1;
}

static int floppy_sense_interrupt(unsigned char *st0, unsigned char *cyl) {
    if (!floppy_send_byte(CMD_SENSE_INT)) return 0;
    if (!floppy_read_byte(st0)) return 0;
    if (!floppy_read_byte(cyl)) return 0;
    return 1;
}

int floppy_init(void) {
    outb(FLOPPY_DOR, 0x00);
    outb(FLOPPY_DOR, 0x0C);
    floppy_wait_irq();
    for (int i = 0; i < 4; i++) {
        unsigned char st0, cyl;
        floppy_sense_interrupt(&st0, &cyl);
    }
    outb(FLOPPY_CCR, 0x00);
    if (!floppy_send_byte(CMD_SPECIFY)) return 0;
    if (!floppy_send_byte(0xDF)) return 0;
    if (!floppy_send_byte(0x02)) return 0;
    if (!floppy_send_byte(CMD_RECALIBRATE)) return 0;
    if (!floppy_send_byte(0x00)) return 0;
    floppy_wait_irq();
    unsigned char st0, cyl;
    if (!floppy_sense_interrupt(&st0, &cyl)) return 0;
    return 1;
}

static void lba_to_chs(unsigned long lba, unsigned char *cyl, unsigned char *head, unsigned char *sector) {
    *cyl = lba / (HEADS * SECTORS_PER_TRACK);
    *head = (lba / SECTORS_PER_TRACK) % HEADS;
    *sector = (lba % SECTORS_PER_TRACK) + 1;
}

static int floppy_seek(unsigned char cyl, unsigned char head) {
    if (!floppy_send_byte(CMD_SEEK)) return 0;
    if (!floppy_send_byte((head << 2) | 0)) return 0;
    if (!floppy_send_byte(cyl)) return 0;
    floppy_wait_irq();
    unsigned char st0, pcyl;
    if (!floppy_sense_interrupt(&st0, &pcyl)) return 0;
    return (pcyl == cyl);
}

int floppy_read_sectors(unsigned long lba, unsigned long count, void *buffer) {
    unsigned char *buf = (unsigned char *)buffer;
    for (unsigned long i = 0; i < count; i++) {
        unsigned char cyl, head, sector;
        lba_to_chs(lba + i, &cyl, &head, &sector);
        if (!floppy_seek(cyl, head)) return 0;
        floppy_setup_dma(0, BYTES_PER_SECTOR);
        if (!floppy_send_byte(CMD_READ_DATA)) return 0;
        if (!floppy_send_byte((head << 2) | 0)) return 0;
        if (!floppy_send_byte(cyl)) return 0;
        if (!floppy_send_byte(head)) return 0;
        if (!floppy_send_byte(sector)) return 0;
        if (!floppy_send_byte(2)) return 0;
        if (!floppy_send_byte(SECTORS_PER_TRACK)) return 0;
        if (!floppy_send_byte(0x1B)) return 0;
        if (!floppy_send_byte(0xFF)) return 0;
        floppy_wait_irq();
        unsigned char st0, st1, st2, rcyl, rhead, rsector, bps;
        if (!floppy_read_byte(&st0)) return 0;
        if (!floppy_read_byte(&st1)) return 0;
        if (!floppy_read_byte(&st2)) return 0;
        if (!floppy_read_byte(&rcyl)) return 0;
        if (!floppy_read_byte(&rhead)) return 0;
        if (!floppy_read_byte(&rsector)) return 0;
        if (!floppy_read_byte(&bps)) return 0;
        for (int j = 0; j < BYTES_PER_SECTOR; j++) {
            buf[i * BYTES_PER_SECTOR + j] = dma_buffer[j];
        }
    }
    return 1;
}

int floppy_write_sectors(unsigned long lba, unsigned long count, const void *buffer) {
    const unsigned char *buf = (const unsigned char *)buffer;
    for (unsigned long i = 0; i < count; i++) {
        unsigned char cyl, head, sector;
        lba_to_chs(lba + i, &cyl, &head, &sector);

        if (!floppy_seek(cyl, head)) return 0;

        for (int j = 0; j < BYTES_PER_SECTOR; j++) {
            dma_buffer[j] = buf[i * BYTES_PER_SECTOR + j];
        }

        floppy_setup_dma(1, BYTES_PER_SECTOR);

        if (!floppy_send_byte(CMD_WRITE_DATA)) return 0;
        if (!floppy_send_byte((head << 2) | 0)) return 0;
        if (!floppy_send_byte(cyl)) return 0;
        if (!floppy_send_byte(head)) return 0;
        if (!floppy_send_byte(sector)) return 0;
        if (!floppy_send_byte(2)) return 0;
        if (!floppy_send_byte(SECTORS_PER_TRACK)) return 0;
        if (!floppy_send_byte(0x1B)) return 0;
        if (!floppy_send_byte(0xFF)) return 0;

        floppy_wait_irq();

        unsigned char st0, st1, st2, rcyl, rhead, rsector, bps;
        if (!floppy_read_byte(&st0)) return 0;
        if (!floppy_read_byte(&st1)) return 0;
        if (!floppy_read_byte(&st2)) return 0;
        if (!floppy_read_byte(&rcyl)) return 0;
        if (!floppy_read_byte(&rhead)) return 0;
        if (!floppy_read_byte(&rsector)) return 0;
        if (!floppy_read_byte(&bps)) return 0;
    }
    return 1;
}
