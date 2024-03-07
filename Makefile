.PHONY: clean all temp 

# CC = ${RISCV}/llvm/build-linux/bin/clang
CC = ${RISCV}/riscv/bin/riscv32-unknown-elf-gcc
AR = ${RISCV}/riscv/bin/riscv32-unknown-elf-ar

CCX86 = gcc
ARX86 = ar

BDIR := build
LDIR := $(BDIR)/lib
IDIR := include
SRC  := src

QEMU-BDIR := build-qemu
QEMU-LDIR := $(QEMU-BDIR)/lib
QEMU-IDIR := $(IDIR)/cx-qemu
QEMU-SRC := $(SRC)/cx-qemu

ZOO-DIR := zoo


cx_objects := $(BDIR)/ci.o $(BDIR)/queue.o $(BDIR)/parser.o
cx_libraries := $(BDIR)/addsub.o $(BDIR)/muldiv.o
cx_helpers := $(QEMU-BDIR)/addsub_func.o $(QEMU-BDIR)/muldiv_func.o
qemu_objects := $(cx_helpers)

all: $(QEMU-LDIR)/libmcfu_selector.so $(LDIR)/libci.so $(cx_libraries)


###########   Qemu functionality   ###########
$(QEMU-LDIR)/libmcfu_selector.so: $(qemu_objects) | $(QEMU-LDIR)
	$(ARX86) -rcs $@ $^

$(QEMU-BDIR)/%.o : $(QEMU-SRC)/%.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-BDIR)/addsub_func.o : $(ZOO-DIR)/addsub/addsub_func.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-BDIR)/muldiv_func.o : $(ZOO-DIR)/muldiv/muldiv_func.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-LDIR):
	mkdir -p $(QEMU-LDIR)


###########   Building cx runtime   ###########
$(LDIR)/libci.so: $(cx_objects) | $(LDIR)
	$(AR) -rcs $@ $(cx_objects)

$(BDIR)/%.o : $(SRC)/%.c | $(LDIR)
	$(CC) -c $< -o $@

$(LDIR):
	mkdir -p $(LDIR)


###########   CX Libraries   ###########
$(BDIR)/addsub.o: $(ZOO-DIR)/addsub/addsub.c $(ZOO-DIR)/addsub/addsub.h
	$(CC) -c $< -o $@

$(BDIR)/muldiv.o: $(ZOO-DIR)/muldiv/muldiv.c $(ZOO-DIR)/muldiv/muldiv.h
	$(CC) -c $< -o $@


###########   Parser   ###########

$(BDIR)/parser.o: $(SRC)/parser.c $(IDIR)/parser.h | $(LDIR)
	$(CC) -c $< -o $@

###########   Building Executeable   ###########
example: examples/example.c
	$(CC) -march=rv32imav -mabi=ilp32 $< $(cx_libraries) -L$(LDIR) -lci -O2 -o example


###########   Running on different emulators   ###########
qemu: example
	${RISCV}/riscv-gnu-toolchain/qemu/build/qemu-riscv32 -L ./utils/riscv/bin/ ./$^


### TODO: Modify spike to execute cx instructions
spike: example
	${RISCV}/riscv-llvm/bin/spike --isa=rv32imav ${RISCV}/riscv-pk/build-llvm/pk $^


###########   Clean build   ###########
clean:
	rm -rf build/
	rm -rf build-qemu/
	rm -f temp