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

#ifndef IDE_H
#define IDE_H

#define IDE_DATA         0x1F0
#define IDE_DRIVE_HEAD   0x1F6
#define IDE_STATUS       0x1F7
#define IDE_COMMAND      0x1F7

#define IDE_CMD_IDENTIFY 0xEC
#define IDE_DRIVE_MASTER 0xA0

extern unsigned short identify_buffer[256];

int ide_init(void);
int ide_detect(void);
void ide_load_identify(void);
void ide_wait_ready(void);
int ide_drive_present(void);

#endif 
