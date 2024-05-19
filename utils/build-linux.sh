# Builds linux for riscv32 with added cx runtime calls
# Based on https://risc-v-getting-started-guide.readthedocs.io/en/latest/linux-qemu.html

git clone https://github.com/torvalds/linux

if [ $? -ne 0 ]; then
	echo "Issue cloning linux repo"
    exit 1
fi

pushd linux
git checkout v6.7

if [ $? -ne 0 ]; then
	echo "Couldn't find correct version of linux kernel (although any version of v6.x should work)"
    exit 1
fi

if [ ! -d cx_sys ]; then
	mkdir cx_sys
fi

# Need to copy cx_open.c from utils dir
cp ../cx_open.c cx_sys/

if [ $? -ne 0 ]; then
	echo "Issue copying cx_open.c to linux directory"
    exit 1
fi

# Makes changes based on the diff
git apply --ignore-space-change --reject --whitespace=fix -C2 ../diffs/cx_linux.diff

if [ $? -ne 0 ]; then
	echo "Issue applying cx diff to linux"
    exit 1
fi

make ARCH=riscv CROSS_COMPILE=riscv32-unknown-linux-gnu- rv32_defconfig
make ARCH=riscv CROSS_COMPILE=riscv32-unknown-linux-gnu- -j $(nproc)

popd
