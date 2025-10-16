#!/bin/sh
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

if [ -f config.conf ]; then
    . ./config.conf
  else
      printf "Error: config.conf not found. Run ./configure.sh first.\n"
      exit 1
fi

HDD_SIZE="1024" # in bytes

CC=gcc
CVER=c99

FLAGS="-ffreestanding \
       -Wall \
       -Wextra \
       -fno-stack-protector \
       -fno-builtin \
       -nostdinc \
       -std=$CVER \
       -pedantic \
       -fno-common \
       -pedantic-errors"

LDFLAGS="-m$BITS \
         -z noexecstack
         -nostdlib \
         -Wl,--build-id=none"

HDD="build/hdd.img"

SRC_DIR=src
BUILD_DIR=build

ELF="cheesedos.elf"
OUTPUT="$BUILD_DIR/$ELF"

BOOT_DIR="$SRC_DIR/boot"

check_dependencies() {
  required_tools="gcc as ld objcopy mkdir rm find basename truncate awk printf test command exit cat sort wait mktemp"
  missing_tools=""

  for tool in $required_tools; do
    printf "Checking for %s..." "$tool"
    if command -v "$tool" > /dev/null 2>&1; then
      printf " Found!\n"
    else
      printf " Not Found!\n"
      if [ -z "$missing_tools" ]; then
        missing_tools="$tool"
      else
        missing_tools="$missing_tools $tool"
      fi
    fi
  done

  if [ -n "$missing_tools" ]; then
    printf "\n"
    printf "SOME TOOLS ARE NOT FOUND!"
    i=1
    for tool in $missing_tools; do
      printf "$i. $tool"
      i=$((i+1))
    done
    exit 1
  fi
}

get_includes() {
  includes=""
  for dir in $(find "$SRC_DIR" -type d | sort); do
    includes="$includes -I$dir"
  done
  printf "$includes"
}

INCLUDES=$(get_includes)

get_source_files() {
  find "$SRC_DIR" -name "*.c" -not -path "$BOOT_DIR/*" | sort
}

get_asm_files() {
  find "$SRC_DIR" -name "*.S" -not -path "$BOOT_DIR/*" | sort
}

get_object_files() {
  objs=""
  
  for src in $(get_source_files); do
    if [ -n "$src" ]; then
      basename_val=$(basename "$src" .c)
      obj="$BUILD_DIR/${basename_val}.o"
      if [ -z "$objs" ]; then
        objs="$obj"
      else
        objs="$objs $obj"
      fi
    fi
  done
  
  for src in $(get_asm_files); do
    if [ -n "$src" ]; then
      basename_val=$(basename "$src" .S)
      obj="$BUILD_DIR/${basename_val}.o"
      if [ -z "$objs" ]; then
        objs="$obj"
      else
        objs="$objs $obj"
      fi
    fi
  done
  
  printf "$objs"
}

CFLAGS="-m$BITS \
        -march=$MARCH \
        -O$OPT \
        -mtune=$MARCH \
        -g$GDBINFO \
        $FLAGS \
        $INCLUDES"

ASMFLAGS="-m$BITS \
          $INCLUDES"

build_c_object() {
  $CC $CFLAGS -c "$1" -o "$2"
}

build_asm_object() {
  $CC $ASMFLAGS -c "$1" -o "$2"
}

all() {
  check_dependencies

  clean

  printf "Making directory: %s..." "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
  printf " Done!\n"

  build_pids=""
  
  build_c() {
    src="$1"
    obj="$2"

    printf "$CC $CFLAGS -c $src -o $obj\n" | awk '{$1=$1; print}'

    {
      output=$(mktemp)
      if build_c_object "$src" "$obj" > "$output" 2>&1; then
        cat "$output"
        rm -f "$output"
      else
        cat "$output"
        rm -f "$output"
        exit 1
      fi
    } &
    if [ -z "$build_pids" ]; then
      build_pids="$!"
    else
      build_pids="$build_pids $!"
    fi
  }
  
  build_asm() {
    src="$1"
    obj="$2"

    printf "$CC $ASMFLAGS -c $src -o $obj" | awk '{$1=$1; print}'

    {
      output=$(mktemp)
      if build_asm_object "$src" "$obj" > "$output" 2>&1; then
        rm -f "$output"
      else
        cat "$output"
        rm -f "$output"
        exit 1
      fi
    } &
    if [ -z "$build_pids" ]; then
      build_pids="$!"
    else
      build_pids="$build_pids $!"
    fi
  }

  for src in $(get_source_files); do
    if [ -n "$src" ]; then
      basename_val=$(basename "$src" .c)
      obj="$BUILD_DIR/${basename_val}.o"
      build_c "$src" "$obj"
    fi
  done
  
  for src in $(get_asm_files); do
    if [ -n "$src" ]; then
      basename_val=$(basename "$src" .S)
      obj="$BUILD_DIR/${basename_val}.o"
      build_asm "$src" "$obj"
    fi
  done

  for pid in $build_pids; do
    wait "$pid"
  done

  printf "Assembling bootloader..."
    $CC -m$BITS -c -o "$BUILD_DIR/boot.o" "$BOOT_DIR/boot.S"
  printf " Done!\n"
  
  printf "Linking bootloader..."
    $CC $LDFLAGS -Wl,-T,"$BOOT_DIR/boot.ld" -Wl,--oformat=binary -o "$BUILD_DIR/boot.bin" "$BUILD_DIR/boot.o"
  printf " Done!\n"
      
  OBJS=$(get_object_files)
  
  obj_count=0
  for obj in $OBJS; do
    obj_count=$((obj_count + 1))
  done
  
  printf "Linking cheeseDOS with %d object files..." "$obj_count"
    $CC $LDFLAGS -Wl,-e,init -Wl,-T,"$SRC_DIR/link/link.ld" -o "$OUTPUT" $OBJS
  printf " Done!\n"

  printf "Stripping %s..." "$OUTPUT"
    objcopy --strip-all --remove-section=.rel.dyn --remove-section=.got.plt $OUTPUT
  printf " Done!\n"

  printf "Building %s..." "$FLOPPY"
    cat "$BUILD_DIR/boot.bin" "$OUTPUT" > "$FLOPPY"
  printf " Done!\n"

  printf "Padding %s..." "$FLOPPY"
    truncate -s 1474560 "$FLOPPY"
  printf " Done!\n"

  exit 0
}

make_hdd_image() {
  printf "Creating %sB disk image to %s..." "$HDD_SIZE" "$HDD"
    dd if=/dev/zero of="$HDD" bs="$HDD_SIZE" count=1 \
      > /dev/null 2>&1
  printf " Done!\n"
}

MEM=1M
CPU=486
CPU_FLAGS="-fpu,-mmx,-sse,-sse2,-sse3,-ssse3,-sse4.1,-sse4.2"

run() {
  if [ ! -f "$FLOPPY" ]; then
    printf "Error: Floppy image $FLOPPY not found. Run 'build' first.\n"
    exit 1
  fi
  
  if [ ! -f "$HDD" ]; then
    make_hdd_image
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

run_kvm() {
  if [ ! -f "$FLOPPY" ]; then
    printf "Error: Floppy image $FLOPPY not found. Run 'build' first.\n"
    exit 1
  fi
  
  if [ ! -f "$HDD" ]; then
    make_hdd_image
  fi

  $SU \
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
    -drive file="$HDD",format=raw,if=ide,media=disk \
    -enable-kvm
}

clean() {
  printf "Cleaning up..."
    rm -rf "$BUILD_DIR" "$FLOPPY" "$ISO_ROOT"
  printf " Done!\n"
}

distclean() {
  printf "Cleaning up..."
    rm -rf "$BUILD_DIR" "$FLOPPY" "$ISO_ROOT" "config.conf"
  printf " Done!\n"
}

case "$1" in
  "") all ;;
  all) all ;;
  run) run ;;
  run-kvm) run_kvm ;;
  clean) clean ;;
  distclean) distclean ;;
  *) printf "Usage: $0 {all|run|run-kvm|clean|distclean}\n" ; exit 1 ;;
esac
