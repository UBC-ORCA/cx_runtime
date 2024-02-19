.PHONY: clean

# CC = ${RISCV}/llvm/build-linux/bin/clang
CC = ${RISCV}/build-linux/bin/riscv32-unknown-elf-gcc
AR = ${RISCV}/build-linux/bin/riscv32-unknown-elf-ar

objects := ci.o queue.o 

all: libci.a

libci.a: $(objects)
	$(AR) -rcs $@ $(objects)

ci.o: ci.c queue.o
	$(CC) -c ci.c

queue.o: queue.c queue.h
	$(CC) -c queue.c

# Building the test file
test: test.c
	$(CC) -march=rv32imav -mabi=ilp32 test.c -lci -L.

# Running on different emulators
spike: a.out
	${RISCV}/riscv-llvm/bin/spike --isa=rv32imav ${RISCV}/riscv-pk/build-llvm/pk $^

qemu: a.out
	${RISCV}/riscv-gnu-toolchain/qemu/build/qemu-riscv32 -L ${RISCV}/sysroot ./$^

# clean the directory
clean:
	rm -f prog *.o *.a