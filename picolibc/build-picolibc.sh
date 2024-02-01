#!/bin/bash

if [ $# -eq 0 ]; then
    >&2 echo "Please provide a directory to install picolibc intoas the first and only argument."
    exit 1
fi

# save current directory
cwd=$(pwd)

# change to specified directory
cd $1

# cd into that directory
git clone -b linker_crt https://github.com/sel4-cap/picolibc.git
cd picolibc
./build_script_microkit.sh

# Copy necessary files
cp picolib-microkit/newlib/libc.a $cwd
cp picolib-microkit/picolibc.specs $cwd