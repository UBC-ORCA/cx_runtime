## Install riscv toolchain and Qemu
1. run `./build-rvv-toolchain.sh`
2. add `RISCV` env variable to bashrc: `export RISCV=/path/to/cx_runtime/utils`

## TODO: Optional - LLVM build

# cx_runtime
1. run `make all`

# Building an example
1. `make test`
2. `make qemu`