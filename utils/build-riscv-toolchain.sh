#!/usr/bin/env bash

#####
# Works on Ubuntu 20.04
# This script builds the rvv toolchain. The invocation looks like:
# build-rvv-toolchain.sh
#
# e.g. ./build-rvv-toolchain.sh
#
# NOTE: This is a very slow build (2h+) due to rvv toolchain
#####

export RISCV=$(pwd)

# riscv toolchain build can fail if these are set
unset LIBRARY_PATH
unset LD_LIBRARY_PATH

# Installs the necessary packages for running spike, pk, riscv-gnu-toolchain
sudo apt-get install autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build
if [ ! -d riscv-gnu-toolchain ]; then
    git clone https://github.com/riscv-collab/riscv-gnu-toolchain riscv-gnu-toolchain
fi

pushd riscv-gnu-toolchain

git checkout 59ab58e8a4aed4ed8f711ebab307757a5ebaa1f5

if [ $? -ne 0 ]; then
	echo "Issue with checking out correct commit"
    exit 1
fi

git submodule init binutils gcc gdb glibc newlib qemu
git submodule update

if [ $? -ne 0 ]; then
	echo "Issue with updating rvv submodules"
    exit 1
fi

pushd binutils

# Makes changes based on the diff
git apply --ignore-space-change --reject --whitespace=fix -C2 ../../binutils.diff

if [ $? -ne 0 ]; then
	echo "Issue applying cx diff to rvv toolchain"
    exit 1
fi

popd

if [ -d build ]; then
    rm -rf build/
fi

mkdir build
pushd build
../configure --prefix=$RISCV/riscv --with-arch=rv32imav --with-abi=ilp32

if [ $? -ne 0 ]; then
	echo "issue with configuring rvv build"
    exit 1
fi

# Note: changing this to have multiple jobs causes the build to fail
make

if [ $? -ne 0 ]; then
	echo "issue with building rvv"
    exit 1
fi

popd

pushd qemu

git checkout f48c205fb42be48e2e47b7e1cd9a2802e5ca17b0

if [ $? -ne 0 ]; then
	echo "Issue with getting correct commit of submodule: qemu"
    exit 1
fi

# Need to test this out - should make changes from the diff
git apply --ignore-space-change --reject --whitespace=fix -C2 ../../qemu_cx.diff

if [ $? -ne 0 ]; then
	echo "Issue with applying diff changes to qemu"
    exit 1
fi

cp ../../cx_helper.c target/riscv/cx_helper.c

if [ $? -ne 0 ]; then
	echo "Issue with copying files to qemu (cx_helper.c)"
    exit 1
fi

cp ../../trans_cx.c.inc target/riscv/insn_trans/trans_cx.c.inc

if [ $? -ne 0 ]; then
	echo "Issue with copying files to qemu (trans_cx.c.inc)"
    exit 1
fi

if [ -d build ]; then
    rm -rf build/
fi

mkdir build
pushd build

export LIBRARY_PATH=$RISCV/../build-qemu/lib
export LD_LIBRARY_PATH=$RISCV/../build-qemu/lib

../configure --target-list=riscv64-linux-user,riscv32-linux-user,riscv64-softmmu,riscv32-softmmu

if [ $? -ne 0 ]; then
	echo "issue with configuring qemu - possibly due to not having a python venv active"
    exit 1
fi

make

if [ $? -ne 0 ]; then
	echo "issue building qemu"
    exit 1
fi

popd
