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
                                                                                             #
.intel_syntax noprefix                                                                       # Use intel syntax
.code16                                                                                      # Use 16 bit real mode
                                                                                             #
.equ STACK_TOP, 0x7C00                                                                       # Top of stack
.equ SELF_LOAD, 0x7C00                                                                       # Location of bootloader
.equ ELF_HDR_LOAD, 0x7E00                                                                    # Where cheeseDOS will be loaded
                                                                                             #
.equ SECT_SIZE, 512                                                                          # Sector size
.equ SECT_SHIFT, 9                                                                           # Number of bits needed to do a bit shift 
.equ GEOM_SECTORS, 18                                                                        # Sectors
.equ GEOM_CYLINDERS, 80                                                                      # Cylinders
.equ GEOM_HEADS, 2                                                                           # Heads
                                                                                             #
.equ elf_phoff, 0x1C                                                                         # Offset from beginning of the ELF file header to start of program header table
.equ elf_phnum, 0x2C                                                                         # Number of entries in PHT
.equ elf_entry, 0x18                                                                         # Virtual memory address of entry point
                                                                                             #
.equ elf_seg_type, 0x00                                                                      # Segment type field offset
.equ elf_seg_file_offset, 0x04                                                               # Segment start offset
.equ elf_seg_paddr, 0x0C                                                                     # Address of the segment
.equ elf_seg_filesz, 0x10                                                                    # Size of segment
.equ elf_seg_memsz, 0x14                                                                     # Size of segment in memory
.equ elf_seg_struct_size, 0x20                                                               # Size of a program header entry
                                                                                             #
.equ gdt_fl_pglimit, 0x80                                                                    # GDT page granularity
.equ gdt_fl_32b, 0x40                                                                        # GDT 32-bit segment
                                                                                             #
.equ gdt_a_present, 0x80                                                                     # GDT present
.equ gdt_a_dpl0, 0x00                                                                        # GDT ring 0
.equ gdt_a_nosys, 0x10                                                                       # GDT non system segment
.equ gdt_a_exec, 0x08                                                                        # GDT executable
.equ gdt_a_conforming, 0x04                                                                  # GDT conforming code
.equ gdt_a_grow_down, 0x04                                                                   # GDT grow down stack
.equ gdt_a_rw, 0x2                                                                           # GDT readable/writable
.equ gdt_a_accessed, 0x1                                                                     # GDT accessed bit
                                                                                             #
.section .text                                                                               # Start of code section
.global start                                                                                # Export entry point
start:                                                                                       # Entry point
                                                                                             #
cli                                                                                          # Disable interrupts
jmp 0:start2                                                                                 # Far jump to flush prefetch queue
start2:                                                                                      # The "real" stuff begins here
                                                                                             #
xor cx, cx                                                                                   # Zero CX
mov ss, cx                                                                                   # Set stack segment to zero
mov sp, STACK_TOP                                                                            # Set stack pointer to begining of the bootloader
mov ds, cx                                                                                   # Set data segment to zero
                                                                                             #
mov ax, 0xB800                                                                               # Video memory segment
mov es, ax                                                                                   # Set ES to video memory
xor di, di                                                                                   # Start at offset 0
mov ah, 0x0F                                                                                 # Change text color to white on black
                                                                                             #
mov al, ' '                                                                                  # Set character to ASCII space
mov cx, 2000                                                                                 # Pixels on screen
.clear_loop:                                                                                 # Screen clear loop
stosw                                                                                        # Store word
loop .clear_loop                                                                             # Repeat for how many pixels are on screen
mov ah, 0x02                                                                                 # Set cursor
xor bh, bh                                                                                   # Target display page zero
xor dh, dh                                                                                   # Set cursor to row zero
xor dl, dl                                                                                   # Set cursor to column 0
int 0x10                                                                                     # Apply changes
                                                                                             #
mov si, offset msg_loading                                                                   # Get msg_loading
call print                                                                                   # Print it out
                                                                                             #
mov si, drives                                                                               # Get drives
                                                                                             #
scan_floppy:                                                                                 # Floppy scan loop
    lodsb                                                                                    # Grab next drive from list
    test al, al                                                                              # Check if drive ID is empty
    jz no_drives                                                                             # Error out if there is no drives
    mov dl, al                                                                               # Tell BIOS what drive to read
                                                                                             #
    mov ax, 0x0201                                                                           # Ask BIOS to read one sector
    mov cx, 0x0002                                                                           # Read sector two
    xor dx, dx                                                                               # Set head to zero
    mov bx, ELF_HDR_LOAD/16                                                                  # Set segment for loading ELF header
    mov es, bx                                                                               # Tell BIOS where to load the data
    xor bx, bx                                                                               # Set offset to zero
    int 0x13                                                                                 # Start reading
    jc scan_floppy                                                                           # If read failed, try next drive 
                                                                                             #
    jmp load_init                                                                            # If succeeded, Load cheeseDOS
                                                                                             #
load_init:                                                                                   # Initialize ELF loading
    mov ax, ELF_HDR_LOAD/16                                                                  # Set segment where the ELF header is loaded
    mov ds, ax                                                                               # Set data segment to the ELF header
    mov si, [elf_phoff]                                                                      # Load program header offset
    mov cl, [elf_phnum]                                                                      # Load number of program headers
                                                                                             #
load_segment:                                                                                # Load segment loop
    push cx                                                                                  # Save value of CX
    mov eax, [si + elf_seg_type]                                                             # Get segment type from ELF header
    dec eax                                                                                  # Check if segment is loadable
    jnz skip_seg                                                                             # If segment is not loadable, skip loading it
                                                                                             #
    mov ebx, [si + elf_seg_paddr]                                                            # Get segment's memory address
    shr ebx, 4                                                                               # Convert the address into a paragraph address
    mov es, bx                                                                               # Load the paragraph address into ES
                                                                                             #
    mov eax, [si + elf_seg_file_offset]                                                      # Get segment's starting offset on the disk
    shr eax, SECT_SHIFT                                                                      # Convert the byte offset to a sector number
    inc ax                                                                                   # Adjust the sector number to be one indexed
                                                                                             #
    push dx                                                                                  # Save value of DX
    xor dx, dx                                                                               # Clear DX for division
    mov bx, GEOM_SECTORS                                                                     # Get the number of sectors per track
    div bx                                                                                   # Divide by sectors per track
    mov cl, dl                                                                               # Store sector number in CL
    inc cl                                                                                   # Make sector one-indexed
    mov bl, GEOM_HEADS                                                                       # Get number of heads
    div bl                                                                                   # Divide to get cylinder and head
    pop dx                                                                                   # Restore original value of DX
    mov dh, ah                                                                               # Store head number in DH
    mov ch, al                                                                               # Store cylinder number in CH
                                                                                             #
    xor ebx, ebx                                                                             # Clear EBX
    mov bx, SECT_SIZE - 1                                                                    # Load sector size minus 1
    mov eax, [si + elf_seg_filesz]                                                           # Get segment size from ELF header
    add eax, ebx                                                                             # Add to round up
    shr eax, SECT_SHIFT                                                                      # Convert bytes to sectors
                                                                                             #
    mov ah, 0x02                                                                             # BIOS read sectors function
    xor bx, bx                                                                               # Set offset to zero
    int 0x13                                                                                 # Call BIOS interrupt
    jc err                                                                                   # Error out if read failed
                                                                                             #
skip_seg:                                                                                    # Skip segment label
    pop cx                                                                                   # Restore CX
    add si, elf_seg_struct_size                                                              # Move to next program header
    loop short load_segment                                                                  # Loop through all segments
                                                                                             #
    mov esi, [elf_entry]                                                                     # Get entry point address
                                                                                             #
    .equ gdtp, 0                                                                             # GDT pointer offset
    .equ gdt, 8                                                                              # GDT offset
    cld                                                                                      # Clear direction flag
    mov ax, ds                                                                               # Copy DS to AX
    mov es, ax                                                                               # Set ES to DS
    xor di, di                                                                               # Start at offset 0
                                                                                             #
    xor eax, eax                                                                             # Clear EAX
    mov al, 3*8 - 1                                                                          # Set GDT limit
    stosw                                                                                    # Store GDT limit
    mov ax, ELF_HDR_LOAD + gdt                                                               # Calculate GDT base address
    stosd                                                                                    # Store lower word of GDT base
    stosw                                                                                    # Store upper word of GDT base
                                                                                             #
    xor eax, eax                                                                             # Clear EAX
    stosd                                                                                    # Store low null descriptor
    stosd                                                                                    # Store high null descriptor
    lea bx, [gdt_code]                                                                       # Load address of code descriptor
    mov eax, cs:[bx]                                                                         # Load first dword of code descriptor
    mov edx, cs:[bx+4]                                                                       # Load second dword of code descriptor
    stosd                                                                                    # Store low code descriptor
    xchg eax, edx                                                                            # Swap registers
    stosd                                                                                    # Store high code descriptor
    xchg eax, edx                                                                            # Swap back
    stosd                                                                                    # Store low data descriptor
    xchg eax, edx                                                                            # Swap registers
    stosd                                                                                    # Store high data descriptor
    mov al, gdt_a_present | gdt_a_nosys | gdt_a_dpl0 | gdt_a_rw | gdt_a_accessed             # Set access flags
    mov ds:[gdt + 8*2 + 5], al                                                               # Set data descriptor access byte
                                                                                             #
    in al, 0x92                                                                              # Read from port 0x92
    or al, 2                                                                                 # Set A20 enable bit
    out 0x92, al                                                                             # Write back to enable A20
                                                                                             #
    lgdt [gdtp]                                                                              # Load GDT
    mov eax, cr0                                                                             # Read CR0
    or eax, 1                                                                                # Set protected mode bit
    mov cr0, eax                                                                             # Enter protected mode
                                                                                             #
    jmp 8:prot32                                                                             # Far jump to flush pipeline
                                                                                             #
.code32                                                                                      # Switch to 32 bit
prot32:                                                                                      # Protected mode entry point
    xor eax, eax                                                                             # Clear EAX
    mov al, 16                                                                               # Load data segment selector
    mov ds, ax                                                                               # Set DS
    mov es, ax                                                                               # Set ES
    mov fs, ax                                                                               # Set FS
    mov gs, ax                                                                               # Set GS
    mov ss, ax                                                                               # Set SS
                                                                                             #
    jmp esi                                                                                  # Jump to cheeseDOS
                                                                                             #
.code16                                                                                      # Switch back to 16 bit
                                                                                             #
print:                                                                                       # Print function
    cld                                                                                      # Clear direction flag
    mov ah, 0x0E                                                                             # BIOS teletype function
    xor bx, bx                                                                               # Page 0, color 0
.loop:                                                                                       # Print loop
    lodsb                                                                                    # Load next character
    test al, al                                                                              # Check for null terminator
    jz .ret                                                                                  # Return if null
    int 0x10                                                                                 # Print character
    jmp .loop                                                                                # Loop to next character
.ret:                                                                                        # Return label
    ret                                                                                      # Return from function
                                                                                             #
no_drives:                                                                                   # No drives error handler
    mov ds, bx                                                                               # Set DS to zero
    mov si, offset msg_no_drives                                                             # Load error message
    call print                                                                               # Print error message
    jmp halt_loop                                                                            # Halt system
                                                                                             #
drives:                                                                                      # Drive list
    .byte 0x00                                                                               # First floppy drive
    .byte 0x01                                                                               # Second floppy drive
    .byte 0x00                                                                               # End of list
                                                                                             #
msg_loading:                                                                                 # Loading message
    .ascii "Starting cheeseDOS...\r\0"                                                       # Loading message text
msg_no_drives:                                                                               # No drives message
    .ascii "Error [19]: cheeseDOS could not find any bootable devices!"                      # No drives message text
                                                                                             #
gdt_code:                                                                                    # Code segment descriptor
    .word 0xFFFF                                                                             # Limit low
    .word 0x0000                                                                             # Base low
    .byte 0x0                                                                                # Base middle
    .byte gdt_a_present | gdt_a_nosys | gdt_a_dpl0 | gdt_a_exec | gdt_a_rw | gdt_a_accessed  # Access byte
    .byte gdt_fl_32b | gdt_fl_pglimit | 0xF                                                  # Flags and limit high
    .byte 0x0                                                                                # Base high
                                                                                             #
halt_loop:                                                                                   # Halt loop
    hlt                                                                                      # Halt CPU
    jmp halt_loop                                                                            # Loop forever
                                                                                             #
.space 0x200 - 2 - (. - start)                                                               # Pad to 510B
.word 0xaa55                                                                                 # Make it bootable
