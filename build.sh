#!/bin/bash
#
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

# CONFIGURATION --------------------------------------------------

SU=sudo # "doas" also work too
CC=gcc # "clang" (sometimes) works too
AS=as
LD=ld

FLOPPY=cdos-1.44mb-floppy.img

HDD_SIZE="1" # in bytes

# "BITS" Options:
# 1. 32
# 2. 64
BITS=32

# "MARCH" Options:
#  1. i386 (default)
#  2. i486
#  3. i586
#  4. i686
#  5. pentium
#  6. pentium-mmx
#  7. pentiumpro
#  8. pentium2
#  9. pentium3
# 10. pentium4
# 11. prescott
# 12. nocona
# 13. core2
# 14. nehalem
# 15. westmere
# 16. sandybridge
# 17. ivybridge
# 18. haswell
# 19. broadwell
# 20. skylake
# 21. cannonlake
# 22. icelake-client
# 23. icelake-server
# 24. tigerlake
# 25. rocketlake
# 26. alderlake
# 27. sapphirerapids
# 28. meteorlake
# 29. emeraldrapids
# 30. graniterapids-d
# 31. arrowlake
# 32. lunarlake
# 33. pantherlake
# 34. siliconrapids
# 35. clearlake
MARCH=i386

# "OPT" Options:
# 1. 0 (none)
# 2. 1
# 3. 2
# 4. 3
# 5. f (fast) (use this when we somehow stop getting all that overlapping .data and .text or whatever junk)
# 6. s (size) (default, because its speedy and doesnt use a lot of memory)
# 7. g (debug)
# 8. z (more small) (experimental)
OPT=s

# "GDBINFO" Options:
# 1. 0 (none) (default)
# 2. 1
# 3. 2
# 4. 3
GDBINFO=0

## Flags for gcc
CVER=99

FLAGS="-ffreestanding \
       -Wall \
       -Wextra \
       -fno-stack-protector \
       -fno-builtin \
       -nostdinc \
       -std=c$CVER"

# END OF CONFIGURATION --------------------------------------------------

HDD="build/hdd.img"

LDFLAGS="-m \
         elf_i386 \
         -z \
         noexecstack"

ISO_ROOT=iso_root

SRC_DIR=src
BUILD_DIR=build

KERNEL="$BUILD_DIR/kernel.elf"

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
  -I$IDE_DIR \
  -I$KERNEL_DIR"

OBJS=(
  "$BUILD_DIR/kernel.o"
  "$BUILD_DIR/shell.o"
  "$BUILD_DIR/vga.o"
  "$BUILD_DIR/keyboard.o"
  "$BUILD_DIR/ramdisk.o"
  "$BUILD_DIR/calc.o"
  "$BUILD_DIR/string.o"
  "$BUILD_DIR/rtc.o"
  "$BUILD_DIR/beep.o"
  "$BUILD_DIR/acpi.o"
  "$BUILD_DIR/timer.o"
  "$BUILD_DIR/programs.o"
  "$BUILD_DIR/serial.o"
  "$BUILD_DIR/version.o"
  "$BUILD_DIR/ide.o"
)
       
CFLAGS="-m$BITS \
        -march=$MARCH \
        -O$OPT \
        -mtune=$MARCH \
        -g$GDBINFO \
        $FLAGS \
        $INCLUDES"

SKIP_DEPS=0
for arg in "$@"; do
  if [[ "$arg" == "--no-dep-check" ]]; then
    SKIP_DEPS=1
  fi
done

build_object() {
  $CC $CFLAGS -c "$1" -o "$2"
}

function all {
  start=$(date +%s%N)

  echo -n "Building cheeseDOS "
  if [[ -f "src/version/version.txt" ]]; then
    cat src/version/version.txt
  else
    echo "version file not found"
  fi

  echo

  clean
  
  echo

  if [[ "$SKIP_DEPS" -eq 0 ]]; then
    deps
    echo
  else
    echo "Skipping dependency check (--no-dep-check)"
    echo
  fi

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
        exit 1
      fi
      rm -f "$output"
    } &
    build_jobs+=($!)
  }

  if [[ -f "$KERNEL_DIR/kernel.c" ]]; then
    build_echo "$KERNEL_DIR/kernel.c"   "$BUILD_DIR/kernel.o"
  else
    echo "Warning: $KERNEL_DIR/kernel.c not found"
  fi
  
  if [[ -f "$SHELL_DIR/shell.c" ]]; then
    build_echo "$SHELL_DIR/shell.c"     "$BUILD_DIR/shell.o"
  else
    echo "Warning: $SHELL_DIR/shell.c not found"
  fi
  
  if [[ -f "$VGA_DIR/vga.c" ]]; then
    build_echo "$VGA_DIR/vga.c"         "$BUILD_DIR/vga.o"
  else
    echo "Warning: $VGA_DIR/vga.c not found"
  fi
  
  if [[ -f "$KEYBRD_DIR/keyboard.c" ]]; then
    build_echo "$KEYBRD_DIR/keyboard.c" "$BUILD_DIR/keyboard.o"
  else
    echo "Warning: $KEYBRD_DIR/keyboard.c not found"
  fi
  
  if [[ -f "$RAMDISK_DIR/ramdisk.c" ]]; then
    build_echo "$RAMDISK_DIR/ramdisk.c" "$BUILD_DIR/ramdisk.o"
  else
    echo "Warning: $RAMDISK_DIR/ramdisk.c not found"
  fi
  
  if [[ -f "$CALC_DIR/calc.c" ]]; then
    build_echo "$CALC_DIR/calc.c"       "$BUILD_DIR/calc.o"
  else
    echo "Warning: $CALC_DIR/calc.c not found"
  fi
  
  if [[ -f "$STRING_DIR/string.c" ]]; then
    build_echo "$STRING_DIR/string.c"   "$BUILD_DIR/string.o"
  else
    echo "Warning: $STRING_DIR/string.c not found"
  fi
  
  if [[ -f "$RTC_DIR/rtc.c" ]]; then
    build_echo "$RTC_DIR/rtc.c"         "$BUILD_DIR/rtc.o"
  else
    echo "Warning: $RTC_DIR/rtc.c not found"
  fi
  
  if [[ -f "$BEEP_DIR/beep.c" ]]; then
    build_echo "$BEEP_DIR/beep.c"       "$BUILD_DIR/beep.o"
  else
    echo "Warning: $BEEP_DIR/beep.c not found"
  fi
  
  if [[ -f "$ACPI_DIR/acpi.c" ]]; then
    build_echo "$ACPI_DIR/acpi.c"       "$BUILD_DIR/acpi.o"
  else
    echo "Warning: $ACPI_DIR/acpi.c not found"
  fi
  
  if [[ -f "$TIMER_DIR/timer.c" ]]; then
    build_echo "$TIMER_DIR/timer.c"     "$BUILD_DIR/timer.o"
  else
    echo "Warning: $TIMER_DIR/timer.c not found"
  fi
  
  if [[ -f "$PROGRAMS_DIR/programs.c" ]]; then
    build_echo "$PROGRAMS_DIR/programs.c" "$BUILD_DIR/programs.o"
  else
    echo "Warning: $PROGRAMS_DIR/programs.c not found"
  fi
  
  if [[ -f "$UART_DIR/serial.c" ]]; then
    build_echo "$UART_DIR/serial.c"     "$BUILD_DIR/serial.o"
  else
    echo "Warning: $UART_DIR/serial.c not found"
  fi
  
  if [[ -f "$IDE_DIR/ide.c" ]]; then
    build_echo "$IDE_DIR/ide.c"     "$BUILD_DIR/ide.o"
  else
    echo "Warning: $IDE_DIR/ide.c not found"
  fi

  for job in "${build_jobs[@]}"; do
    wait "$job"
  done

  echo -n "Building version.o..."
  if [[ -f "$VER_DIR/version.txt" ]]; then
    objcopy -I binary -O elf$BITS-i386 -B i386 \
            "$VER_DIR/version.txt" "$BUILD_DIR/version.o"
    echo " Done!"
  else
    echo " Skipped (version.txt not found)"
  fi

  echo

  if [[ -f "$BOOT_DIR/boot.S" ]]; then
    echo -n "Assembling bootloader..."
    $AS --32 -I "$BOOT_DIR" -o "$BUILD_DIR/boot.o" "$BOOT_DIR/boot.S"
    echo " Done!"
    
    if [[ -f "$BOOT_DIR/boot.ld" ]]; then
      echo -n "Linking bootloader..."
      $LD $LDFLAGS -T "$BOOT_DIR/boot.ld" -o "$BUILD_DIR/boot.elf" "$BUILD_DIR/boot.o"
      echo " Done!"
      
      echo -n "Converting boot.elf into boot.bin..."
      objcopy -O binary -j .text "$BUILD_DIR/boot.elf" "$BUILD_DIR/boot.bin"
      echo " Done!"
    else
      echo "Error: $BOOT_DIR/boot.ld not found"
      exit 1
    fi
  else
    echo "Error: $BOOT_DIR/boot.S not found"
    exit 1
  fi
  
  echo

  if [[ -f "$KERNEL_DIR/kernel.ld" ]]; then
    echo -n "Linking kernel..."
    $LD $LDFLAGS -e kmain -z max-page-size=512 -T "$KERNEL_DIR/kernel.ld" -o "$KERNEL" "${OBJS[@]}"
    echo " Done!"
    
    echo -n "Stripping kernel..."
    strip -sv "$KERNEL" > /dev/null 2>&1
    echo " Done!"
  else
    echo "Error: $KERNEL_DIR/kernel.ld not found"
    exit 1
  fi
  
  echo

  echo -n "Building $FLOPPY..."
  cat "$BUILD_DIR/boot.bin" "$KERNEL" > "$FLOPPY"
  echo " Done!"
    
  echo -n "Pad $FLOPPY..."
  truncate "$FLOPPY" -s '1474560'
  echo " Done!"

  echo

  echo -n "Creating ${HDD_SIZE}B disk image to $HDD..."
  dd if=/dev/zero of="$HDD" bs="$HDD_SIZE" count=1 \
  > /dev/null 2>&1
  echo " Done!"

  echo

  end=$(date +%s%N)
  elapsed_ns=$((end - start))
  elapsed_sec=$(printf "%d.%03d\n" $((elapsed_ns / 1000000000)) $(((elapsed_ns / 1000000) % 1000)))

  echo "Build completed, made floppy at $FLOPPY in $elapsed_sec seconds."

  exit 0
}

MEM=1M
CPU=486
CPU_FLAGS="-fpu,-mmx,-sse,-sse2,-sse3,-ssse3,-sse4.1,-sse4.2"

function run {
  if [[ ! -f "$FLOPPY" ]]; then
    echo "Error: Floppy image $FLOPPY not found. Run 'build' first."
    exit 1
  fi
  
  qemu-system-i386 \
  -audiodev pa,id=snd0 \
  -machine pcspk-audiodev=snd0 \
  -serial stdio \
  -drive file="$FLOPPY",format=raw,if=floppy \
  -m "$MEM" \
  -cpu "$CPU","$CPU_FLAGS" \
  -vga std \
  -display gtk \
  -rtc base=localtime \
  -nodefaults \
  -drive file="$HDD",format=raw,if=ide,media=disk
}

function write {
  if [[ ! -f "$FLOPPY" ]]; then
    echo "Error: Floppy image $FLOPPY not found. Run 'build' first."
    exit 1
  fi
  
  lsblk
  read -p "Enter target device (e.g. fd0): " dev
  echo "Writing to /dev/$dev ..."
  $SU dd if="$FLOPPY" of="/dev/$dev" bs=512 conv=notrunc status=progress && sync
  echo "Done!"
}

function deps {
  echo "Checking for required tools: gcc, binutils, and qemu-system-i386..."

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
        [qemu-system-i386]="qemu-system-x86"
      )
      ;;
    dnf)
      pkg_map=(
        [gcc]="gcc"
        [ld]="binutils"
        [qemu-system-i386]="qemu-system-x86"
      )
      ;;
    zypper)
      pkg_map=(
        [gcc]="gcc"
        [ld]="binutils"
        [qemu-system-i386]="qemu"
      )
      ;;
    pacman)
      missing=()
      if ! command -v gcc &> /dev/null; then missing+=("gcc"); fi
      if ! command -v ld &> /dev/null; then missing+=("binutils"); fi
      if ! command -v qemu-system-i386 &> /dev/null; then missing+=("qemu-arch-extra"); fi
      
      if [ ${#missing[@]} -eq 0 ]; then
        echo "All dependencies are already installed."
        return 0
      fi
      
      echo "Missing: ${missing[*]}"
      echo "Attempting to install missing dependencies..."
      sudo pacman -Syu --noconfirm
      sudo pacman -S --noconfirm "${missing[@]}"
      return 0
      ;;
    emerge)
      pkg_map=(
        [gcc]="sys-devel/gcc"
        [ld]="sys-devel/binutils"
        [qemu-system-i386]="app-emulation/qemu"
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
    emerge)
      for pkg in "${missing[@]}"; do
        sudo emerge "$pkg"
      done
      ;;
  esac
}

function clean {
  echo -n "Cleaning up: $BUILD_DIR $FLOPPY $ISO_ROOT..."
  rm -rf "$BUILD_DIR" "$FLOPPY" "$ISO_ROOT"
  echo " Done!"
}

case "$1" in
  "") all ;;
  all) all ;;
  run) run ;;
  write) write ;;
  clean) clean ;;
  *) echo "Usage: $0 {all|run|write|clean} [--no-dep-check]" ; exit 1 ;;
esac
