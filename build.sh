#!/bin/bash


export MICROKIT_BOARD="maaxboard"
export MICROKIT_CONFIG="debug"
export BUILD_DIR="./build"
export SOURCE_LOCATION=$(pwd)

example_list=("empty_client" "moving_square" "resolution_change" "rotating_bars" "static_image") 

if [ -z "$1" ]; then
    echo "Please use one of the following examples as the first argument to this script. E.g ./build.sh empty_client"
    echo "${example_list[*]}"
    exit 1
fi

EXAMPLE=$1

if [[ " ${example_list[*]} " != *"$EXAMPLE"* ]];
then
    echo "Example doesn't exist Please use one of the following examples as the first argument to this script. E.g ./build.sh empty_client"
    echo "${example_list[*]}"
    exit 1
fi


if [ $1 = "empty_client" ] ; then
    export CURRENT_EXAMPLE="$1"
else
    export CURRENT_EXAMPLE="examples/$1"
fi 


# Set up for current development switching between two environments. Use no second argument if using bje docker configuration
# See https://github.com/sel4-cap/microkit-maaxboard-dockerfiles

if [ -z "$2" ]; then

    export MICROKIT_SDK="/util/microkit/sel4_foundation/release/microkit-sdk-1.2.6"
    export PATH="/util/sel4_foundation_arm_toolchain_baseline/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin:$PATH"
    
    export BOARD_DIR="$MICROKIT_SDK/board/maaxboard/debug/"
    export PYTHONPATH="$MICROKIT_SDK/bin"
    export MICROKIT_TOOL=" $MICROKIT_SDK/bin/microkit"


elif [ $2 = "c" ] ; then

    export MICROKIT_DIR="$HOME/dev/microkit/mk-manifest/microkit"
    export MICROKIT_SDK="$MICROKIT_DIR/release/microkit-sdk-1.2.6"
    export PATH="$MICROKIT_DIR/../compiler/bin:$PATH"
    
    export BOARD_DIR="$MICROKIT_SDK/board/$MICROKIT_BOARD/$MICROKIT_CONFIG"
    export PYTHONPATH="$MICROKIT_DIR/tool"
    export MICROKIT_TOOL="python -m microkit"

else
    echo "Invalid argument - Leave empty if building inside docker container, and 'c' if using own configuration"
    exit 1
fi


# Confirm picolibc files exist
if  test -e picolibc/libc.a  &&  test -e picolibc/picolibc.specs ; then
    echo "Found picolib build files"
else
    echo "Can't find picolib build files. Place libc.a and picolib.specs from your picolib build into the picolib directory.
Alternatively run the picolib/build-picolib.sh script to build picolib."
    exit
fi


# Make clean 
rm -r $BUILD_DIR/*
mkdir build
make -C $SOURCE_LOCATION

# Set tftp location depending on build environment
if [ -z "$1" ]; then
    TFTP_LOCATION="/srv/tftp/loader-tom.img"    # TODO:FIX for bje docker configuration
else 
    TFTP_LOCATION="/var/lib/tftpboot/loader-tom.img"
fi

# Copy image file to tftp location 
sudo cp $BUILD_DIR/loader.img $TFTP_LOCATION
if [ $? -gt 0 ]; then
    echo "Error(s) detected, file not copied"
else
    echo Copied image file to "$TFTP_LOCATION".
fi