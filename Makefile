# cheeseDOS - My x86 DOS
# Copyright (C) 2025  Connor Thomson
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

CC = gcc
AS = as
LD = ld
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -fno-builtin-strcpy -fno-builtin-strncpy
LDFLAGS = -m elf_i386 -T link.ld
KERNEL = kernel.elf
ISO = cdos.iso
ISO_DIR = iso
BOOT_DIR = $(ISO_DIR)/boot
GRUB_DIR = $(BOOT_DIR)/grub
GRUB_CFG = grub.cfg
OBJS = boot.o kernel.o shell.o vga.o keyboard.o ramdisk.o calc.o string.o rtc.o

RM = rm -f
RMDIR = rm -rf

all: $(ISO)

boot.o: boot.S
	$(AS) --32 -o $@ $<

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c $< -o $@

vga.o: vga.c vga.h
	$(CC) $(CFLAGS) -c $< -o $@

keyboard.o: keyboard.c keyboard.h
	$(CC) $(CFLAGS) -c $< -o $@

ramdisk.o: ramdisk.c ramdisk.h
	$(CC) $(CFLAGS) -c $< -o $@

calc.o: calc.c calc.h
	$(CC) $(CFLAGS) -c $< -o $@

string.o: string.c string.h
	$(CC) $(CFLAGS) -c $< -o $@

rtc.o: rtc.c rtc.h io.h
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS) link.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(ISO): $(KERNEL)
	mkdir -p $(GRUB_DIR)
	cp $(GRUB_CFG) $(GRUB_DIR)
	cp $(KERNEL) $(BOOT_DIR)/
	grub-mkrescue -o $@ $(ISO_DIR)

write: $(ISO)
	@lsblk
	@read -p "Enter target device (e.g. sdb): " dev; \
	echo "Writing to /dev/$$dev ..."; \
	sudo dd if=cdos.iso of=/dev/$$dev bs=4M status=progress && sync

run: $(ISO)
	qemu-system-i386 $<

run64: $(ISO)
	qemu-system-x86_64 $<

clean:
	$(RMDIR) $(ISO_DIR)
	$(RM) $(ISO) $(KERNEL) $(OBJS)

.PHONY: all clean write run run64
