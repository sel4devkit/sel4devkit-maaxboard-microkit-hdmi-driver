#!/bin/bash

# NOTE: This script must be run from the same directory "sel4-hdmi/picolibc"

# save current directory
cwd=$(pwd)

# cd into that directory
git clone -b linker_crt https://github.com/sel4-cap/picolibc.git
cd picolibc
./build_script_microkit.sh

# Copy necessary files
sudo cp picolib-microkit/newlib/libc.a $cwd
sudo cp picolib-microkit/picolibc.specs $cwd