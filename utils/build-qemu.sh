if [ ! -d qemu ]; then
    git clone https://github.com/qemu/qemu.git
fi

if [ $? -ne 0 ]; then
	echo "Issue cloning qemu"
    exit 1
fi

pushd qemu

git checkout f48c205fb42be48e2e47b7e1cd9a2802e5ca17b0

if [ $? -ne 0 ]; then
	echo "Issue with getting correct commit of submodule: qemu"
    exit 1
fi

# Need to test this out - should make changes from the diff
git apply --ignore-space-change --reject --whitespace=fix -C2 ../../diffs/qemu_cx.diff

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

../configure --target-list=riscv32-softmmu

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
