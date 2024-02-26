## Install Riscv Toolchain and Qemu
1. run `make all`
1. activate a python virtual env (mamba, venv, conda, etc.) - needed for Qemu
2. run `./build-rvv-toolchain.sh`
3. add `RISCV` env variable to bashrc: `export RISCV=/path/to/cx_runtime/utils`

## TODO: Optional - LLVM build

# cx_runtime
1. run `make all` again.

# Building an example
1. `make test`
2. `make qemu`