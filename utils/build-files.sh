if [ ! -d initramfs/home ]; then
    exit 1
fi

pushd initramfs
pushd home

riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/cx_open.c -o cx_open -L $CX_ROOT/build/lib/ -lci
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/state_test.c -o state -L $CX_ROOT/build/lib/ -lci
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/stateless.c -o sl -L $CX_ROOT/build/lib/ -lci
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/intra_virt.c -o iv -L $CX_ROOT/build/lib/ -lci
riscv32-unknown-linux-gnu-gcc -static `pwd`/../../../examples/threaded_test.c -o tt -L $CX_ROOT/build/lib/ -lci -pthread

popd
find . -print0 | cpio --null -ov --format=newc | gzip -9 > initramfs.cpio.gz
popd
