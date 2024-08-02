#!/bin/bash

# This work is Crown Copyright NCSC, 2024.

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


# Use no second argument if using follwoing docker configuration:
# https://github.com/sel4-cap/sel4devkit-maaxboard-docker-dev-env

if [ -z "$2" ]; then

    export MICROKIT_SDK="/packages/microkit/sel4_microkit/microkit-sdk-1.3.0/"
    export BOARD_DIR="$MICROKIT_SDK/board/maaxboard/debug/"
    export PYTHONPATH="$MICROKIT_SDK/bin"
    export MICROKIT_TOOL=" $MICROKIT_SDK/bin/microkit"

# Example of a custom configuration
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