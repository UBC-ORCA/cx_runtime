## Install Riscv Toolchain and Qemu
1. From the `cx_runtime/` dir, run `make all`
2. activate a python virtual env (mamba, venv, conda, etc.) - needed for Qemu
3. run `./build-rvv-toolchain.sh` from `cx_runtime/utils/`
4. add `RISCV` env variable to bashrc: `export RISCV=/path/to/cx_runtime/utils`

# cx_runtime
1. run `make all` again, from the `cx_runtime/` dir.

# Building an example
1. `make test`
2. `make qemu`

## TODO: Optional - LLVM build
