#!/bin/bash
export MICROKIT_BOARD="maaxboard"
export MICROKIT_CONFIG="debug"
export MICROKIT_TOOL="python -m microkit"

export MICROKIT_DIR="$HOME/dev/microkit/mk-manifest/microkit" #! change this to microkit directory (should this default be somewhere else, or is it fine here?)
export MICROKIT_SDK="$MICROKIT_DIR/release/microkit-sdk-1.2.6"
export BOARD_DIR="$MICROKIT_SDK/board/$MICROKIT_BOARD/$MICROKIT_CONFIG"
export BUILD_DIR="./build"

export PYTHONPATH="$MICROKIT_DIR/tool"
export PATH="$MICROKIT_DIR/../compiler/bin:$PATH"

export SOURCE_LOCATION=$(pwd)
# export NETBSD_DIR="$SEL4_XHCI_PATH/netbsd/src"

if  test -e picolibc/libc.a  &&  test -e picolibc/picolibc.specs ; then
    echo "Found picolib build files"
else
    echo "Can't find picolib build files. Place libc.a and picolib.specs from your picolib build into the picolib directory.
Alternatively run the picolib/build-picolib.sh script to build picolib."
    exit
fi

# make clean

rm -r $BUILD_DIR/*
mkdir build
make -C $SOURCE_LOCATION
# if clean

# specific to capgemini implementation
echo 
TFTP_LOCATION="/var/lib/tftpboot/loader-tom.img"
sudo cp $BUILD_DIR/loader.img $TFTP_LOCATION
if [ $? -gt 0 ]; then
    echo "Error(s) detected, file not copied"
else
    echo Copied image file to "$TFTP_LOCATION".
fi