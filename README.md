# cx_runtime

* A user example of an M-Mode program can be found in examples/example.c

* Make sure to build the rvv toolchain and source the settings file in cva5pr before continuing!

* To build the m mode cx library, run `make machine`

# Adding a CXU
To add a CXU, do the following (following crc32 or rgb2luma as examples).
1. Add a `<CXU>_common.h` file in `zoo/<CXU>/`
2. Add the cxu to cx_init() in `include/ci_m.h`, being sure to include the new common file in `include/ci_m.h`
3. Run `make machine` to update the library
