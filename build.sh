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

# CONFIGURATION --------------------------------------------------

# `doas` works too
SU="sudo"

FLOPPY=cdos-1.44mb-floppy.img

HDD_SIZE="1024" # in bytes

# "BITS" Options:
# 1. 32
# 2. 64
#
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
#
MARCH=i386

# "OPT" Options:
# 1. 0 (none)
# 2. 1
# 3. 2
# 4. 3
# 5. fast (use this when we somehow stop getting all that overlapping .data and .text or whatever junk)
# 6. s (size) (default, because it doesn't use a lot of memory)
# 7. g (debug)
# 8. z (more small) (experimental)
#
OPT=s

# "GDBINFO" Options:
# 1. 0 (none) (default)
# 2. 1
# 3. 2
# 4. 3
#
GDBINFO=0

# "STRIP" Options:
# 1. true (default)
# 2. false
#
STRIP=true

# Version of C to use
#
# "CVER" Options:
# 1. c99
# 2. c11
# 3. c18
# 4. c23
#
CVER=c99

# Flags for gcc
#
# Used flags with meanings:
# 1. "-freestanding" - Use freestanding environment
# 2. "-Wall" - Enable all warnings
# 3. "-Wextra" - Enable extra warnings
# 4. "-fno-stack-protector" - Disable stack protector
# 5. "-fno-builtin" - Disable built-in functions
# 6. "-nostdinc" - Disable standard include paths
# 7. "-std=$CVER" - Version of C to use
# 8. "-pedantic" - Disable GNU extensions (disable if you are using any gnuXX C version)
# 9. "-fno-common" - Disable common symbols
# 10. "-pedantic-errors" - Treat all warnings as errors
#
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

# Flags for ld:
#
# Used flags with meanings:
# 1. "-m" - Specify the target architecture
# 2. "elf_i386" - Specify the output format
# 3. "-z" - Set linker options
# 4. "noexecstack" - Mark the stack as non-executable
#
LDFLAGS="-m \
         elf_i386 \
         -z \
         noexecstack"

# END OF CONFIGURATION --------------------------------------------------

CC=gcc
AS=as
LD=ld

HDD="build/hdd.img"

SRC_DIR=src
BUILD_DIR=build

OUTPUT="$BUILD_DIR/cheesedos.elf"
BOOT_DIR="$SRC_DIR/boot"

check_dependencies() {
  required_tools="gcc objcopy as ld strip truncate"
  missing_tools=""

  for tool in $required_tools; do
    printf "Checking for %s..." "$tool"
    if command -v "$tool" > /dev/null 2>&1; then
      echo " Found!"
    else
      echo " Not Found!"
      if [ -z "$missing_tools" ]; then
        missing_tools="$tool"
      else
        missing_tools="$missing_tools $tool"
      fi
    fi
  done

  if [ -n "$missing_tools" ]; then
    echo
    echo "SOME TOOLS ARE NOT FOUND!"
    i=1
    for tool in $missing_tools; do
      echo "$i. $tool"
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
  echo "$includes"
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
  
  if [ -z "$objs" ]; then
    objs="$BUILD_DIR/version.o"
  else
    objs="$objs $BUILD_DIR/version.o"
  fi
  
  echo "$objs"
}

CFLAGS="-m$BITS \
        -march=$MARCH \
        -O$OPT \
        -mtune=$MARCH \
        -g$GDBINFO \
        $FLAGS \
        $INCLUDES"

ASMFLAGS="--$BITS $INCLUDES"

build_c_object() {
  $CC $CFLAGS -c "$1" -o "$2"
}

build_asm_object() {
  $AS $ASMFLAGS -o "$2" "$1"
}

all() {
  start=$(date +%s)

  echo "Building cheeseDOS $(cat src/version/version.txt)..."

  echo

  check_dependencies

  clean
  
  printf "Making directory: %s..." "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
  echo " Done!"

  build_pids=""
  
  build_c() {
    src="$1"
    obj="$2"

    echo "$CC $CFLAGS -c $src -o $obj" | awk '{$1=$1; print}'

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

    echo "$AS $ASMFLAGS -o $obj $src" | awk '{$1=$1; print}'

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

  printf "Building version.o..."
    objcopy -I binary -O elf$BITS-i386 -B i386 \
      "$SRC_DIR/version/version.txt" "$BUILD_DIR/version.o"
  echo " Done!"

  printf "Assembling bootloader..."
    $AS --32 -I "$BOOT_DIR" -o "$BUILD_DIR/boot.o" "$BOOT_DIR/boot.S"
  echo " Done!"
  
  printf "Linking bootloader..."
    $LD $LDFLAGS -T "$BOOT_DIR/boot.ld" -o "$BUILD_DIR/boot.elf" "$BUILD_DIR/boot.o"
  echo " Done!"
      
  printf "Converting boot.elf into boot.bin..."
    objcopy -O binary -j .text "$BUILD_DIR/boot.elf" "$BUILD_DIR/boot.bin"
  echo " Done!"

  OBJS=$(get_object_files)
  
  obj_count=0
  for obj in $OBJS; do
    obj_count=$((obj_count + 1))
  done
  
  printf "Linking cheeseDOS with %d object files..." "$obj_count"
    $LD $LDFLAGS -e init -z max-page-size=512 -T "$SRC_DIR/link/link.ld" -o "$OUTPUT" $OBJS
  echo " Done!"

  if [ "$STRIP" = "true" ]; then
    printf "Stripping debug symbols..."
     strip -s "$OUTPUT"
    echo " Done!"
  else
    echo "Keeping debug symbols (STRIP=false)"
  fi
    
  printf "Building %s..." "$FLOPPY"
    cat "$BUILD_DIR/boot.bin" "$OUTPUT" > "$FLOPPY"
  echo " Done!"

  printf "Pad %s from %s to 1.44MB..." "$FLOPPY" "$(du -BK "$FLOPPY" | cut -f1)"
    truncate "$FLOPPY" -s '1474560'
  echo " Done!"

  echo

  end=$(date +%s)
  elapsed_sec=$((end - start))

  echo "Build completed, made floppy at $FLOPPY in $elapsed_sec seconds."

  exit 0
}

make_hdd_image() {
  printf "Creating %sB disk image to %s..." "$HDD_SIZE" "$HDD"
    dd if=/dev/zero of="$HDD" bs="$HDD_SIZE" count=1 \
      > /dev/null 2>&1
  echo " Done!"
}

MEM=1M
CPU=486
CPU_FLAGS="-fpu,-mmx,-sse,-sse2,-sse3,-ssse3,-sse4.1,-sse4.2"

run() {
  if [ ! -f "$FLOPPY" ]; then
    echo "Error: Floppy image $FLOPPY not found. Run 'build' first."
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
    echo "Error: Floppy image $FLOPPY not found. Run 'build' first."
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
  printf "Cleaning up: %s %s %s..." "$BUILD_DIR" "$FLOPPY" "$ISO_ROOT"
    rm -rf "$BUILD_DIR" "$FLOPPY" "$ISO_ROOT"
  echo " Done!"
}

case "$1" in
  "") all ;;
  all) all ;;
  run) run ;;
  run-kvm) run_kvm ;;
  clean) clean ;;
  *) echo "Usage: $0 {all|run|run-kvm|clean}" ; exit 1 ;;
esac