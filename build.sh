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

SRC_DIR=src
BUILD_DIR=build

KERNEL="$BUILD_DIR/kernel.elf"

BANNER_DIR="$SRC_DIR/banner"
BOOT_DIR="$SRC_DIR/boot"
CALC_DIR="$SRC_DIR/calc"
DRIVERS_DIR="$SRC_DIR/drivers"
ACPI_DIR="$DRIVERS_DIR/acpi"
BEEP_DIR="$DRIVERS_DIR/beep"
KEYBRD_DIR="$DRIVERS_DIR/keyboard"
VGA_DIR="$DRIVERS_DIR/vga"
KERNEL_DIR="$SRC_DIR/kernel"
RAMDISK_DIR="$SRC_DIR/ramdisk"
SHELL_DIR="$SRC_DIR/shell"
PROGRAMS_DIR="$SRC_DIR/programs"
LIB_DIR="$SRC_DIR/libraries"
STRING_DIR="$LIB_DIR/string"
RTC_DIR="$SRC_DIR/rtc"
TIMER_DIR="$SRC_DIR/timer"

INCLUDES="-I$KERNEL_DIR \
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
  -I$PROGRAMS_DIR"

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
)

BITS=32 # 32 is backwards compatible with 64 but not vice versa and also don't change?
MARCH=i386 # i386 is 32-bit and is backward compatible with i486, i586, i686 and most x86_64 cpus
OPT=fast # Change to 2 or 3 if you have stability issues

FLAGS="-ffreestanding \
       -Wall \
       -Wextra \
       -fno-stack-protector \
       -fno-builtin-strcpy \
       -fno-builtin-strncpy"

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
  echo The cheeseDOS Build System
  echo
  echo BITS=$BITS
  echo MARCH=$MARCH
  echo OPT=$OPT
  echo

  clean
  mkdir -p "$BUILD_DIR"
  echo Made directory: "$BUILD_DIR"
  echo

  build_object "$KERNEL_DIR/kernel.c" "$BUILD_DIR/kernel.o"
  echo Built kernel.o
  build_object "$SHELL_DIR/shell.c" "$BUILD_DIR/shell.o"
  echo Built shell.o
  build_object "$VGA_DIR/vga.c" "$BUILD_DIR/vga.o"
  echo Built vga.o
  build_object "$KEYBRD_DIR/keyboard.c" "$BUILD_DIR/keyboard.o"
  echo Built keyboard.o
  build_object "$RAMDISK_DIR/ramdisk.c" "$BUILD_DIR/ramdisk.o"
  echo Built ramdisk.o
  build_object "$CALC_DIR/calc.c" "$BUILD_DIR/calc.o"
  echo Built calc.o
  build_object "$STRING_DIR/string.c" "$BUILD_DIR/string.o"
  echo Built string.o
  build_object "$RTC_DIR/rtc.c" "$BUILD_DIR/rtc.o"
  echo Built rtc.o
  build_object "$BEEP_DIR/beep.c" "$BUILD_DIR/beep.o"
  echo Built beep.o
  build_object "$ACPI_DIR/acpi.c" "$BUILD_DIR/acpi.o"
  echo Built acpi.o
  build_object "$TIMER_DIR/timer.c" "$BUILD_DIR/timer.o"
  echo Built timer.o
  build_object "$PROGRAMS_DIR/programs.c" "$BUILD_DIR/programs.o"
  echo Built programs.o

  objcopy -I binary -O elf32-i386 -B i386 \
          "$BANNER_DIR/banner.txt" "$BUILD_DIR/banner.o"
  echo Built banner.o

  echo
  $AS --32 -I "$BOOT_DIR" -o "$BUILD_DIR/loader.o" "$BOOT_DIR/loader.S"
  echo Assembled cheeseLDR
  $LD $LDFLAGS -T "$BOOT_DIR/loader.ld" -o "$BUILD_DIR/loader.elf" "$BUILD_DIR/loader.o"
  echo Linked cheeseLDR
  objcopy -O binary -j .text "$BUILD_DIR/loader.elf" "$BUILD_DIR/loader.bin"
  echo "Converted loader.elf to loader.bin"
  echo

  $LD $LDFLAGS -e kmain -z max-page-size=512 -T "$KERNEL_DIR/kernel.ld" -o "$KERNEL" "${OBJS[@]}"
  echo Linked kernel
  strip -sv "$KERNEL"
  echo Stripped kernel

  if test "${BUILD_FLOPPY:-1}" -eq 1; then
    cat "$BUILD_DIR/loader.bin" "$KERNEL" > "$FLOPPY"
    echo "Made $FLOPPY with kernel"
    truncate "$FLOPPY" -s '1474560'
    echo
    echo "Added padding to $FLOPPY"  
  fi

  echo
  rm -rf "$BUILD_DIR"
  echo "Cleaned up: "$BUILD_DIR""
  
  printf "\nBuild completed, Floppy image is $FLOPPY\n" "$elapsed"
}

MEM=1M
CPU=486

function run {
  qemu-system-$MARCH \
  -audiodev pa,id=snd0 \
  -machine pcspk-audiodev=snd0 \
  -fda $FLOPPY \
  -m $MEM \
  -cpu $CPU
}

function write {
  lsblk
  read -p "Enter target device (e.g. fd0): " dev
  echo "Writing to /dev/$dev ..."
  $SU dd if="$FLOPPY" of="/dev/$dev" bs=512 conv=notrunc status=progress && sync
  echo Done!
}

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

function clean {
  rm -rf "$BUILD_DIR" "$FLOPPY"
  echo "Cleaned up: "$BUILD_DIR" "$FLOPPY""
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