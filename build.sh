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

CC=gcc
AS=as
LD=ld
SU=sudo

INCLUDES="-Isrc/kernel \
-Isrc/kernel/shell \
-Isrc/kernel/ramdisk \
-Isrc/drivers \
-Isrc/drivers/vga \
-Isrc/drivers/keyboard \
-Isrc/libraries/string \
-Isrc/calc \
-Isrc/rtc \
-Isrc/banner"

CFLAGS="-m32 -ffreestanding -O2 -Wall -Wextra \
-fno-stack-protector -fno-builtin-strcpy -fno-builtin-strncpy \
-march=i386 $INCLUDES"

LDFLAGS="-m elf_i386 -z noexecstack"

BUILD_DIR=build
KERNEL="$BUILD_DIR/kernel.elf"
KERNEL_DBG="$BUILD_DIR/kernel.debug.elf"
FLOPPY="cdos.img"
GRUB_CFG=src/boot/grub.cfg

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
)

build_object() {
  $CC $CFLAGS -c "$1" -o "$2"
}

function all {
  clean
  mkdir -p "$BUILD_DIR"

  build_object src/kernel/kernel.c "$BUILD_DIR/kernel.o"
  build_object src/kernel/shell/shell.c "$BUILD_DIR/shell.o"
  build_object src/drivers/vga/vga.c "$BUILD_DIR/vga.o"
  build_object src/drivers/keyboard/keyboard.c "$BUILD_DIR/keyboard.o"
  build_object src/kernel/ramdisk/ramdisk.c "$BUILD_DIR/ramdisk.o"
  build_object src/calc/calc.c "$BUILD_DIR/calc.o"
  build_object src/libraries/string/string.c "$BUILD_DIR/string.o"
  build_object src/rtc/rtc.c "$BUILD_DIR/rtc.o"
  
  objcopy -I binary -O elf32-i386 -B i386 \
          src/banner/banner.txt "$BUILD_DIR/banner.o"

  $AS --32 -I src/boot -o "$BUILD_DIR/loader.o" src/boot/loader.S
  $LD $LDFLAGS -T src/boot/loader.ld -o "$BUILD_DIR/loader.elf" "$BUILD_DIR/loader.o"
  objcopy -O binary -j .text "$BUILD_DIR/loader.elf" "$BUILD_DIR/loader.bin"

  $LD $LDFLAGS -e kmain -z max-page-size=512 -T src/kernel/kernel.ld -o "$KERNEL" "${OBJS[@]}"
  objcopy --only-keep-debug "$KERNEL" "$KERNEL_DBG"
  strip -sv "$KERNEL"

  if test "${BUILD_FLOPPY:-1}" -eq 1; then
    cat "$BUILD_DIR/loader.bin" "$KERNEL" > "$FLOPPY"
    truncate "$FLOPPY" -s '1474560' # This makes QEMU assume the correct geometry
  fi

  rm -rf "$BUILD_DIR"

  echo Done!
}

function run {
  qemu-system-i386 -fda $FLOPPY -m 1M -cpu 486
}

#function write {
#  lsblk
#  read -p "Enter target device (e.g. sdb): " dev
#  echo "Writing to /dev/$dev ..."
#  $SU dd if="$ISO" of="/dev/$dev" bs=4M status=progress && sync
#}

function deps {
  if command -v apt &> /dev/null; then
    echo "Detected apt-based system. Installing dependencies..."
    $SU apt-get update && $SU apt-get install -y qemu-system-x86 qemu gcc gcc-multilib binutils
  elif command -v dnf &> /dev/null; then
    echo "Detected dnf-based system. Installing dependencies..."
    $SU dnf update -y && $SU dnf install -y qemu-system-x86 qemu gcc gcc-g++ gcc-toolset binutils
  elif command -v zypper &> /dev/null; then
    echo "Detected SUSE-based system. Installing dependencies..."
    $SU zypper refresh && $SU zypper install -y qemu-x86 gcc gcc-32bit binutils
  elif command -v pacman &> /dev/null; then
    echo "Detected Arch-based system. Installing dependencies..."
    $SU pacman -Syu --noconfirm && $SU pacman -S --noconfirm qemu gcc multilib-devel binutils
  elif command -v portage &> /dev/null; then
    echo "Detected Gentoo-based system. Installing dependencies..."
    $SU emerge --sync && $SU emerge app-emulation/qemu sys-devel/gcc sys-devel/binutils
  else
    echo "Your distro is not supported by cheeseDOS, please see: https://github.com/The-cheeseDOS-Project/cheeseDOS/wiki/Build-and-Run#prerequisites"
  fi
}

#function burn {
#  echo "Burning '$ISO' to /dev/sr0..."
#  $SU wodim dev=/dev/sr0 -v -data "$ISO"
#}

function clean {
  rm -rf "$BUILD_DIR" "$FLOPPY"
}

case "$1" in
  "") all ;;
  all) all ;;
  build) build ;;
  run) run ;;
  write) write ;;
  deps) deps ;;
  burn) burn ;;
  clean) clean ;;
  *) echo "Usage: $0 {all|build|run|clean|deps}" ;;
esac
