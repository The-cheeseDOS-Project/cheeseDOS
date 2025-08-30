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

# CONFIGURATION --------------------------------------------------

# `doas` works too
SU="sudo"

FLOPPY=cdos-1.44mb-floppy.img

HDD_SIZE="1" # in bytes

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
# 5. f (fast) (use this when we somehow stop getting all that overlapping .data and .text or whatever junk)
# 6. s (size) (default, because its speedy and doesnt use a lot of memory)
# 7. g (debug)
# 8. z (more small) (experimental)
#
OPT=s

# "GDBINFO" Options:#
# 1. 0 (none) (default)
# 2. 1
# 3. 2
# 4. 3
#
GDBINFO=0

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

get_includes() {
  local includes=""
  while IFS= read -r -d '' dir; do
    includes="$includes -I$dir"
  done < <(find "$SRC_DIR" -type d -print0)
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
  local objs=()
  
  while IFS= read -r src; do
    if [[ -n "$src" ]]; then
      local basename=$(basename "$src" .c)
      local obj="$BUILD_DIR/${basename}.o"
      objs+=("$obj")
    fi
  done < <(get_source_files)
  
  while IFS= read -r src; do
    if [[ -n "$src" ]]; then
      local basename=$(basename "$src" .S)
      local obj="$BUILD_DIR/${basename}.o"
      objs+=("$obj")
    fi
  done < <(get_asm_files)
  
  objs+=("$BUILD_DIR/version.o")
  
  printf '%s\n' "${objs[@]}"
}

CFLAGS="-m$BITS \
        -march=$MARCH \
        -O$OPT \
        -mtune=$MARCH \
        -g$GDBINFO \
        $FLAGS \
        $INCLUDES"

ASMFLAGS="--$BITS $INCLUDES"

SKIP_DEPS=0
for arg in "$@"; do
  if [[ "$arg" == "--no-dep-check" ]]; then
    SKIP_DEPS=1
  fi
done

build_c_object() {
  $CC $CFLAGS -c "$1" -o "$2"
}

build_asm_object() {
  $AS $ASMFLAGS -o "$2" "$1"
}

function all {
  start=$(date +%s%N)

  echo "Building cheeseDOS $(<src/version/version.txt)..."

  echo

  clean
  
  if [[ "$SKIP_DEPS" -eq 0 ]]; then
    deps
  else
    echo "Skipping dependency check (--no-dep-check)"
  fi

  echo -n "Making directory: $BUILD_DIR..."
  mkdir -p "$BUILD_DIR"
  echo " Done!"

  build_jobs=()
  
  build_c() {
    local src="$1"
    local obj="$2"

    echo "$CC $CFLAGS -c $src -o $obj" | awk '{$1=$1; print}'

  {
    output=$(mktemp)
    build_c_object "$src" "$obj" >"$output" 2>&1
    cat "$output"
    grep -q "error:" "$output" && exit 1
    rm -f "$output"
  } &
    build_jobs+=($!)
  }
  
  build_asm() {
    local src="$1"
    local obj="$2"

    echo "$AS $ASMFLAGS -o $obj $src" | awk '{$1=$1; print}'

    {
      output=$(mktemp)
      build_asm_object "$src" "$obj" >"$output" 2>&1 || { cat "$output"; exit 1; }
      rm -f "$output"
    } &
    build_jobs+=($!)
  }

  while IFS= read -r src; do
    if [[ -n "$src" ]]; then
      local basename=$(basename "$src" .c)
      local obj="$BUILD_DIR/${basename}.o"
      build_c "$src" "$obj"
    fi
  done < <(get_source_files)
  
  while IFS= read -r src; do
    if [[ -n "$src" ]]; then
      local basename=$(basename "$src" .S)
      local obj="$BUILD_DIR/${basename}.o"
      build_asm "$src" "$obj"
    fi
  done < <(get_asm_files)

  for job in "${build_jobs[@]}"; do
    wait "$job"
  done

  if [[ -f "$SRC_DIR/version/version.txt" ]]; then
    echo -n "Building version.o..."
      objcopy -I binary -O elf$BITS-i386 -B i386 \
              "$SRC_DIR/version/version.txt" "$BUILD_DIR/version.o"
    echo " Done!"
  fi

  echo -n "Assembling bootloader..."
    $AS --32 -I "$BOOT_DIR" -o "$BUILD_DIR/boot.o" "$BOOT_DIR/boot.S"
  echo " Done!"
  
  echo -n "Linking bootloader..."
    $LD $LDFLAGS -T "$BOOT_DIR/boot.ld" -o "$BUILD_DIR/boot.elf" "$BUILD_DIR/boot.o"
  echo " Done!"
      
  echo -n "Converting boot.elf into boot.bin..."
    objcopy -O binary -j .text "$BUILD_DIR/boot.elf" "$BUILD_DIR/boot.bin"
  echo " Done!"

  mapfile -t OBJS < <(get_object_files)
  
  echo -n "Linking cheeseDOS with $(printf '%s ' "${OBJS[@]}" | wc -w) object files..."
    $LD $LDFLAGS -e init -z max-page-size=512 -T "$SRC_DIR/link/link.ld" -o "$OUTPUT" "${OBJS[@]}"
  echo " Done!"
  
  echo -n "Building $FLOPPY..."
    cat "$BUILD_DIR/boot.bin" "$OUTPUT" > "$FLOPPY"
  echo " Done!"

  echo -n "Pad $FLOPPY from $(du -BK "$FLOPPY" | cut -f1) to 1.44MB..."
    truncate "$FLOPPY" -s '1474560'
  echo " Done!"

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

function run-kvm {
  if [[ ! -f "$FLOPPY" ]]; then
    echo "Error: Floppy image $FLOPPY not found. Run 'build' first."
    exit 1
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
  run-kvm) run-kvm ;;
  clean) clean ;;
  *) echo "Usage: $0 {all|run|run-kvm|clean} [--no-dep-check]" ; exit 1 ;;
esac
