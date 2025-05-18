#!/bin/bash
cd "$(dirname "$0")"

# Unpack arguments
for arg in "$@"; do declare $arg='1'; done

# Default arguments
if [ ! -v release ] && [ ! -v debug ]; then
    echo "Build mode not specified, defaulting to \`debug\`"
    debug=1
fi

if [ ! -v clang ] && [ ! -v gcc ]; then
    echo "Compiler not specified, defaulting to \`clang\`"
    clang=1
fi

# Compile stage
clang_common="-I../src -g $(sdl2-config --cflags)"
gcc_common="-I../src -g $(sdl2-config --cflags)"

clang_debug="clang $clang_common -O0 -DBUILD_DEBUG=1"
gcc_debug="gcc $gcc_common -O0 -DBUILD_DEBUG=1"

clang_release="clang $clang_common -O2"
gcc_release="gcc $gcc_common -O2"

# Link stage
clang_link="$(sdl2-config --libs)"
gcc_link="$(sdl2-config --libs)"
clang_out="-o"
gcc_out="-o"

# Choose compile/link lines
if [ -v clang ]; then
    compile_debug=$clang_debug
    compile_release=$clang_release
    compile_link=$clang_link
    out=$clang_out
fi

if [ -v gcc ]; then
    compile_debug=$gcc_debug
    compile_release=$gcc_release
    compile_link=$gcc_link
    out=$gcc_out
fi

if [ -v debug ]; then compile=$compile_debug; fi
if [ -v release ]; then compile=$compile_release; fi

# Build program
mkdir -p build
cd build
$compile ../src/main.c $compile_link $out chip8

if [ $? -ne 0 ]; then
    echo Build failed!
else
    echo Build succeeded.
fi
