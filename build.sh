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
CC=gcc # Don't change!
AS=as # Don't change!!
LD=ld # Don't change!!!

FLOPPY=cdos.img
CDROM=NULL
ISO_ROOT=iso_root

SRC_DIR=src
BUILD_DIR=build

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
  -I$VER_DIR"

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
)

BITS=32 # 32 is backwards compatible with 64 but not vice versa and also don't change?
MARCH=i386 # i386 is 32-bit and is backward compatible with i486, i586, i686 and most x86_64 cpus
OPT=3 # 0 Does not work for some reason, use 2 if you get a lot of crashes.

FLAGS="-ffreestanding \
       -Wall \
       -Wextra \
       -fno-stack-protector \
       -fno-builtin \
       -nostdinc \
       -fno-pic \
       -mno-red-zone \
       -Wl,--build-id=none \
       -fomit-frame-pointer \
       -fno-unwind-tables \
       -fno-asynchronous-unwind-tables \
       -fno-ident \
       -Wl,--gc-sections \
       -fdata-sections \
       -ffunction-sections \
       -Wl,-s \
       -malign-double \
       -frename-registers \
       -fweb \
       -fno-stack-check \
       -fno-zero-initialized-in-bss \
       -fno-common \
       -fstrict-aliasing \
       -Wl,--section-start=.text=0x1000 \
       -Wl,--no-undefined \
       -falign-functions=16 \
       -falign-loops=16 \
       -falign-jumps=16 \
       -fno-inline-small-functions \
       -fno-inline-functions-called-once \
       -fno-schedule-insns2 \
       -fno-unroll-loops \
       -fmerge-all-constants \
       -fno-ident"
       
CFLAGS="-m$BITS \
        -march=$MARCH \
        -O$OPT \
        $FLAGS \
        $INCLUDES"

LDFLAGS="-m \
         elf_$MARCH \
         -z \
         noexecstack"

build_object() {
  $CC $CFLAGS -c "$1" -o "$2"
}

function all {
  start=$(date +%s%N)

  echo The cheeseDOS Build System
  echo
  echo BITS=$BITS
  echo MARCH=$MARCH
  echo OPT=$OPT
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

  for job in "${build_jobs[@]}"; do
    wait "$job"
  done

  echo -n "Building version.o..."
  objcopy -I binary -O elf$BITS-$MARCH -B $MARCH \
          "$VER_DIR/version.txt" "$BUILD_DIR/version.o"
  echo " Done!"

  echo -n "Building banner.o..."
  objcopy -I binary -O elf$BITS-$MARCH -B $MARCH \
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

 if test "${BUILD_FLOPPY:-1}" -eq 1; then
    echo -n "Building $FLOPPY..."
    cat "$BUILD_DIR/boot.bin" "$KERNEL" > "$FLOPPY"
    echo " Done!"
    
    echo -n "Pad $FLOPPY..." # Remove when autodetect disk geometry is added
    truncate "$FLOPPY" -s '1474560' # Remove when autodetect disk geometry is added
    echo " Done!" # Remove when autodetect disk geometry is added
 fi  
  echo

#  echo -n "Cleaning up..."
#  rm -rf "$BUILD_DIR" "$ISO_ROOT"
#  echo " Done!"
  
#  echo

  end=$(date +%s%N)
  elapsed_ns=$((end - start))
  elapsed_sec=$(printf "%d.%03d\n" $((elapsed_ns / 1000000000)) $(((elapsed_ns / 1000000) % 1000)))

  echo "Build completed, made floppy at $FLOPPY in $elapsed_sec seconds."
}

MEM=1M
CPU=486

function run {
  qemu-system-$MARCH \
  -audiodev pa,id=snd0 \
  -machine pcspk-audiodev=snd0 \
  -serial stdio \
  -drive format=raw,file="$FLOPPY",index=0,if=floppy \
  -m "$MEM" \
  -cpu "$CPU"
}

function write {
  lsblk
  read -p "Enter target device (e.g. fd0): " dev
  echo "Writing to /dev/$dev ..."
  $SU dd if="$FLOPPY" of="/dev/$dev" bs=512 conv=notrunc status=progress && sync
  echo Done!
}

function deps {
  echo "Checking for required tools: gcc, binutils, qemu-system-x86_64..."

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
        [gcc]="gcc"
        [ld]="binutils"
        [qemu-system-x86_64]="qemu-system-x86"
      )
      ;;
    dnf)
      pkg_map=(
        [gcc]="gcc"
        [ld]="binutils"
        [qemu-system-x86_64]="qemu-system-x86"
      )
      ;;
    zypper)
      pkg_map=(
        [gcc]="gcc"
        [ld]="binutils"
        [qemu-system-x86_64]="qemu"
      )
      ;;
    pacman)
      pkg_map=(
        [gcc]="gcc"
        [ld]="binutils"
        [qemu-system-x86_64]="qemu"
      )
      ;;
    emerge)
      pkg_map=(
        [gcc]="sys-devel/gcc"
        [ld]="sys-devel/binutils"
        [qemu-system-x86_64]="app-emulation/qemu"
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
  write) write ;;
  burn) burn ;;
  clean) clean ;;
  *) echo "Usage: $0 {all|run|write|clean}" ;;
esac
