#ifndef FLOPPY_H
#define FLOPPY_H

#define FLOPPY_DOR   0x3F2
#define FLOPPY_MSR   0x3F4
#define FLOPPY_FIFO  0x3F5
#define FLOPPY_CCR   0x3F7

#define CMD_READ_DATA    0xE6
#define CMD_WRITE_DATA   0xC5
#define CMD_SPECIFY      0x03
#define CMD_SENSE_INT    0x08
#define CMD_RECALIBRATE  0x07
#define CMD_SEEK         0x0F

#define MSR_RQM  0x80
#define MSR_DIO  0x40
#define MSR_BUSY 0x10

#define FLOPPY_DMA_CHANNEL 2

#define SECTORS_PER_TRACK 18
#define HEADS 2
#define TRACKS 80
#define BYTES_PER_SECTOR 512

int floppy_init(void);
int floppy_read_sectors(unsigned long lba, unsigned long count, void *buffer);
int floppy_write_sectors(unsigned long lba, unsigned long count, const void *buffer);

#endif
