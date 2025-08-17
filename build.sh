#!/bin/bash

# cheeseDOS - My x86 DOS
# Copyright (C) 2025  Connor Thomson
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

set -e

SU=sudo # Change to doas if you dont use sudo
CC=clang # Don't change!
AS=as # Don't change!!
LD=ld # Don't change!!!

FLOPPY=cdos-1.44mb-floppy.img
CDROM=cheesedos-cdrom.iso
ISO_ROOT=iso_root

SRC_DIR=src
BUILD_DIR=build

HDD="$BUILD_DIR/hdd.img"
HDD_SIZE="512"

KERNEL="$BUILD_DIR/kernel.elf"

BANNER_DIR="$SRC_DIR/banner"
BOOT_DIR="$SRC_DIR/boot"
CALC_DIR="$SRC_DIR/calc"
DRIVERS_DIR="$SRC_DIR/drivers"
ACPI_DIR="$DRIVERS_DIR/acpi"
BEEP_DIR="$DRIVERS_DIR/beep"
IO_DIR="$DRIVERS_DIR/io"
KEYBRD_DIR="$DRIVERS_DIR/keyboard"
UART_DIR="$DRIVERS_DIR/serial"
VGA_DIR="$DRIVERS_DIR/vga"
KERNEL_DIR="$SRC_DIR/kernel"
RAMDISK_DIR="$SRC_DIR/ramdisk"
SHELL_DIR="$SRC_DIR/shell"
PROGRAMS_DIR="$SRC_DIR/programs"
LIB_DIR="$SRC_DIR/libraries"
STDBOOL_DIR="$LIB_DIR/stdbool"
STDDEF_DIR="$LIB_DIR/stddef"
STDINT_DIR="$LIB_DIR/stdint"
STRING_DIR="$LIB_DIR/string"
RTC_DIR="$SRC_DIR/rtc"
TIMER_DIR="$SRC_DIR/timer"
VER_DIR="$SRC_DIR/version"
IDE_DIR="$DRIVERS_DIR/ide"

INCLUDES=" \
  -I$KERNEL_DIR \
  -I$SHELL_DIR \
  -I$RAMDISK_DIR \
  -I$DRIVERS_DIR \
  -I$VGA_DIR \
  -I$KEYBRD_DIR \
  -I$STRING_DIR \
  -I$CALC_DIR \
  -I$RTC_DIR \
  -I$BANNER_DIR \
  -I$BEEP_DIR \
  -I$ACPI_DIR \
  -I$TIMER_DIR \
  -I$PROGRAMS_DIR \
  -I$STDDEF_DIR \
  -I$STDINT_DIR \
  -I$UART_DIR \
  -I$STDBOOL_DIR \
  -I$IO_DIR \
  -I$VER_DIR \
  -I$IDE_DIR"

OBJS=(
  "$BUILD_DIR/kernel.o"
  "$BUILD_DIR/shell.o"
  "$BUILD_DIR/vga.o"
  "$BUILD_DIR/keyboard.o"
  "$BUILD_DIR/ramdisk.o"
  "$BUILD_DIR/calc.o"
  "$BUILD_DIR/string.o"
  "$BUILD_DIR/rtc.o"
  "$BUILD_DIR/banner.o"
  "$BUILD_DIR/beep.o"
  "$BUILD_DIR/acpi.o"
  "$BUILD_DIR/timer.o"
  "$BUILD_DIR/programs.o"
  "$BUILD_DIR/serial.o"
  "$BUILD_DIR/version.o"
  "$BUILD_DIR/ide.o"
)

BITS=32
MARCH=i386
OPT=2
MTUNE=$MARCH
GDBINFO=0

FLAGS="-ffreestanding \
       -Wall \
       -Wextra \
       -fno-stack-protector \
       -fno-builtin \
       -nostdinc"
       
CFLAGS="-m$BITS \
        -march=$MARCH \
        -O$OPT \
        -mtune=$MTUNE \
        -g$GDBINFO
        $FLAGS \
        $INCLUDES"

LDFLAGS="-m \
         elf_i386 \
         -z \
         noexecstack"

build_object() {
  $CC $CFLAGS -c "$1" -o "$2"
}

function all {
  start=$(date +%s%N)

  echo -n "Building cheeseDOS "
  cat src/version/version.txt

  echo

  clean
  
  echo

  deps

  echo

  echo -n "Making directory: $BUILD_DIR..."
  mkdir -p "$BUILD_DIR"
  echo " Done!"

  echo

  build_jobs=()

  build_echo() {
    src="$1"
    obj="$2"
    name="$(basename "$obj")"

    {
      output=$(mktemp)
      if build_object "$src" "$obj" >"$output" 2>&1; then
        echo "Building $name... Done!"
      else
        echo "Building $name... Failed!"
        cat "$output"
      fi
      rm -f "$output"
    } &
    build_jobs+=($!)
  }

  build_echo "$KERNEL_DIR/kernel.c"   "$BUILD_DIR/kernel.o"
  build_echo "$SHELL_DIR/shell.c"     "$BUILD_DIR/shell.o"
  build_echo "$VGA_DIR/vga.c"         "$BUILD_DIR/vga.o"
  build_echo "$KEYBRD_DIR/keyboard.c" "$BUILD_DIR/keyboard.o"
  build_echo "$RAMDISK_DIR/ramdisk.c" "$BUILD_DIR/ramdisk.o"
  build_echo "$CALC_DIR/calc.c"       "$BUILD_DIR/calc.o"
  build_echo "$STRING_DIR/string.c"   "$BUILD_DIR/string.o"
  build_echo "$RTC_DIR/rtc.c"         "$BUILD_DIR/rtc.o"
  build_echo "$BEEP_DIR/beep.c"       "$BUILD_DIR/beep.o"
  build_echo "$ACPI_DIR/acpi.c"       "$BUILD_DIR/acpi.o"
  build_echo "$TIMER_DIR/timer.c"     "$BUILD_DIR/timer.o"
  build_echo "$PROGRAMS_DIR/programs.c" "$BUILD_DIR/programs.o"
  build_echo "$UART_DIR/serial.c"     "$BUILD_DIR/serial.o"
  build_echo "$IDE_DIR/ide.c"     "$BUILD_DIR/ide.o"

  for job in "${build_jobs[@]}"; do
    wait "$job"
  done

  echo -n "Building version.o..."
  objcopy -I binary -O elf$BITS-i386 -B i386 \
          "$VER_DIR/version.txt" "$BUILD_DIR/version.o"
  echo " Done!"

  echo -n "Building banner.o..."
  objcopy -I binary -O elf$BITS-i386 -B i386 \
          "$BANNER_DIR/banner.txt" "$BUILD_DIR/banner.o"
  echo " Done!"

  echo

  echo -n "Assembling bootloader..."
  $AS --32 -I "$BOOT_DIR" -o "$BUILD_DIR/boot.o" "$BOOT_DIR/boot.S"
  echo " Done!"
  
  echo -n "Linking bootloader..."
  $LD $LDFLAGS -T "$BOOT_DIR/boot.ld" -o "$BUILD_DIR/boot.elf" "$BUILD_DIR/boot.o"
  echo " Done!"
  
  echo -n "Converting boot.elf into boot.bin..."
  objcopy -O binary -j .text "$BUILD_DIR/boot.elf" "$BUILD_DIR/boot.bin"
  echo " Done!"
  
  echo

  echo -n "Linking kernel..."
  $LD $LDFLAGS -e kmain -z max-page-size=512 -T "$KERNEL_DIR/kernel.ld" -o "$KERNEL" "${OBJS[@]}"
  echo " Done!"
  
  echo -n "Stripping kernel..."
  strip -sv "$KERNEL" > /dev/null 2>&1
  echo " Done!"
  
  echo

  echo -n "Building $FLOPPY..."
  cat "$BUILD_DIR/boot.bin" "$KERNEL" > "$FLOPPY"
  echo " Done!"
    
  echo -n "Pad $FLOPPY..."
  truncate "$FLOPPY" -s '1474560'
  echo " Done!"

  echo

  echo -n "Building $CDROM..."
  xorriso -as mkisofs \
    -o $CDROM \
    -b $FLOPPY \
    -c boot.cat \
    -boot-load-size 2880 \
    -boot-info-table \
  . \
  > /dev/null 2>&1
  echo " Done!"

  echo -n "Creating "$HDD_SIZE"B disk image to $HDD..."
  dd if=/dev/zero of=$HDD bs=$HDD_SIZE count=1 \
  > /dev/null 2>&1
  echo " Done!"

  echo

  end=$(date +%s%N)
  elapsed_ns=$((end - start))
  elapsed_sec=$(printf "%d.%03d\n" $((elapsed_ns / 1000000000)) $(((elapsed_ns / 1000000) % 1000)))

  echo "Build completed, made floppy at $FLOPPY and CD-ROM at $CDROM in $elapsed_sec seconds."
}

MEM=1M
CPU=486
CPU_FLAGS="-fpu,-mmx,-sse,-sse2,-sse3,-ssse3,-sse4.1,-sse4.2"

function run {
  qemu-system-i386 \
  -audiodev pa,id=snd0 \
  -machine pcspk-audiodev=snd0 \
  -serial stdio \
  -drive file="$FLOPPY",format=raw,if=floppy \
  -drive file=$HDD,format=raw,if=ide,media=disk \
  -m "$MEM" \
  -cpu "$CPU","$CPU_FLAGS" \
  -vga std \
  -display gtk \
  -rtc base=localtime \
  -nodefaults
}

function runcd {
  qemu-system-i386 \
  -audiodev pa,id=snd0 \
  -machine pcspk-audiodev=snd0 \
  -serial stdio \
  -cdrom $CDROM \
  -m "$MEM" \
  -cpu "$CPU","$CPU_FLAGS"
}

function write {
  lsblk
  read -p "Enter target device (e.g. fd0): " dev
  echo "Writing to /dev/$dev ..."
  $SU dd if="$FLOPPY" of="/dev/$dev" bs=512 conv=notrunc status=progress && sync
  echo Done!
}

function deps {
  echo "Checking for required tools: clang, binutils, qemu-system-x86_64 and xorriso..."

  if command -v apt &> /dev/null; then
    pkg_mgr="apt"
  elif command -v dnf &> /dev/null; then
    pkg_mgr="dnf"
  elif command -v zypper &> /dev/null; then
    pkg_mgr="zypper"
  elif command -v pacman &> /dev/null; then
    pkg_mgr="pacman"
  elif command -v emerge &> /dev/null; then
    pkg_mgr="emerge"
  else
    echo "Unsupported distro. Please install manually."
    return 1
  fi

  declare -A pkg_map
  case "$pkg_mgr" in
    apt)
      pkg_map=(
        [clang]="clang"
        [ld]="binutils"
        [qemu-system-x86_64]="qemu-system-x86"
        [xorriso]="xorriso"
      )
      ;;
    dnf)
      pkg_map=(
        [clang]="clang"
        [ld]="binutils"
        [qemu-system-x86_64]="qemu-system-x86"
        [xorriso]="xorriso"
      )
      ;;
    zypper)
      pkg_map=(
        [clang]="llvm-clang"
        [ld]="binutils"
        [qemu-system-x86_64]="qemu"
        [xorriso]="xorriso"
      )
      ;;
    pacman)
      pkg_map=(
        [clang]="clang"
        [ld]="binutils"
        [qemu-system-x86_64]="qemu"
        [xorriso]="xorriso"
      )
      ;;
    emerge)
      pkg_map=(
        [clang]="sys-devel/clang"
        [ld]="sys-devel/binutils"
        [qemu-system-x86_64]="app-emulation/qemu"
        [xorriso]="xorriso"
      )
      ;;
  esac

  missing=()
  for cmd in "${!pkg_map[@]}"; do
    if ! command -v "$cmd" &> /dev/null; then
      missing+=("${pkg_map[$cmd]}")
    fi
  done

  if [ ${#missing[@]} -eq 0 ]; then
    echo "All dependencies are already installed."
    return 0
  fi

  echo "Missing: ${missing[*]}"
  echo "Attempting to install missing dependencies..."

  case "$pkg_mgr" in
    apt)
      sudo apt-get update
      sudo apt-get install -y "${missing[@]}"
      ;;
    dnf)
      install_list=()
      for pkg in "${missing[@]}"; do
        if dnf list --quiet available "$pkg" &> /dev/null; then
          install_list+=("$pkg")
        else
          echo "Package not found in repos: $pkg (skipping)"
        fi
      done
      if [ ${#install_list[@]} -gt 0 ]; then
        sudo dnf install -y "${install_list[@]}"
      else
        echo "No installable missing packages found."
      fi
      ;;
    zypper)
      sudo zypper install -y "${missing[@]}"
      ;;
    pacman)
      sudo pacman -Syu --noconfirm
      sudo pacman -S --noconfirm "${missing[@]}"
      ;;
    emerge)
      for pkg in "${missing[@]}"; do
        sudo emerge "$pkg"
      done
      ;;
  esac
}

function clean {
  echo -n "Cleaning up: "$BUILD_DIR" "$FLOPPY" "$CDROM" "$ISO_ROOT"..."
  rm -rf "$BUILD_DIR" "$FLOPPY" "$CDROM" "$ISO_ROOT"
  echo " Done!"
}

case "$1" in
  "") all ;;
  all) all ;;
  run) run ;;
  runcd) runcd ;;
  write) write ;;
  clean) clean ;;
  *) echo "Usage: $0 {all|run|runcd|write|clean}" ;;
esac
