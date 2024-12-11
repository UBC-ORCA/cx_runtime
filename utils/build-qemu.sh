## Untested...

pushd ..

export LIBRARY_PATH=$CX_ROOT/build-qemu/lib
export LD_LIBRARY_PATH=$CX_ROOT/build-qemu/lib

pushd qemu_cx

if [ $? -ne 0 ]; then
	echo "Couldn't find qemu_cx directory."
    exit 1
fi

mkdir build
pushd build

if [ $? -ne 0 ]; then
	echo "Couldn't find new build directory."
    exit 1
fi

../configure --target-list=riscv32-softmmu

if [ $? -ne 0 ]; then
	echo "issue with configuring qemu - possibly due to not having a python venv active"
    exit 1
fi

ninja

if [ $? -ne 0 ]; then
	echo "issue building qemu"
    exit 1
fi

popd
