## TODO: Make build_all.sh file + instructions on how to build everything

1. run `./build-rvv-toolchain.sh`
2. make the changes to the rvv/binutils as in `binutils.diff`
3. rebuild

# cx_runtime
1. define `${RISCV}` env var to... somewhere
2. run `make all`

# qemu
1. clone qemu
2. checkout commit TODO: FILL IN COMMIT
3. make changes as in qemu_cfu.diff
4. activate a python venv (required for configure)
5. ```
      mkdir build 
      cd build; 
      ../configure --target-list=riscv64-linux-user,riscv32-linux-user,riscv64-softmmu,riscv32-softmmu
      ```

# Building an example
1. `make test`
2. `make qemu`