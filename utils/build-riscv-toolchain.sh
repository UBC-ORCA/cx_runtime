#!/usr/bin/env bash

#####
# NOTE: This is a very slow build (2h+) due to gnu toolchain
#####

unset LIBRARY_PATH
unset LD_LIBRARY_PATH

pushd $CX_ROOT/utils/riscv-gnu-toolchain

sudo apt-get install autoconf automake autotools-dev curl python3 python3-pip python3-tomli libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build git cmake libglib2.0-dev libslirp-dev

git submodule update --init gcc gdb glibc linux-headers newlib dejagnu

if [ $? -ne 0 ]; then
    echo "issue with cloning riscv toolchain submodules"
    exit 1
fi

rm -rf binutils

git clone git@github.com:UBC-ORCA/binutils.git

if [ $? -ne 0 ]; then
    echo "issue with cloning binutils"
    exit 1
fi

pushd binutils

# Poor way to check to see if the binutils was cloned, as it has given several errors in the past
if [ -f SECURITY.txt]; then
    echo "issue with clone of binutils"
    exit 1
fi

popd

if [ -d build ]; then
    rm -rf build/
fi

mkdir build
pushd build
../configure --prefix=$CX_ROOT/utils/riscv --with-arch=rv32imav --with-abi=ilp32

if [ $? -ne 0 ]; then
	echo "issue with configuring gnu build"
    exit 1
fi

# Note: changing this to have multiple jobs causes the build to fail
make linux

if [ $? -ne 0 ]; then
	echo "issue with building gnu"
    exit 1
fi

popd

popd
