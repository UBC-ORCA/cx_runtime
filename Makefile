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


cx_objects := $(BDIR)/ci.o $(BDIR)/queue.o $(BDIR)/parser.o
qemu_objects := $(QEMU-BDIR)/parser.o $(QEMU-BDIR)/mcfu_select.o $(QEMU-BDIR)/addsub_func.o $(QEMU-BDIR)/muldiv_func.o
cx_libraries := $(BDIR)/addsub.o $(BDIR)/muldiv.o

all: $(QEMU-LDIR)/libmcfu_selector.so $(LDIR)/libci.so $(cx_libraries)


###########   Qemu functionality   ###########
$(QEMU-LDIR)/libmcfu_selector.so: $(qemu_objects) $(QEMU-BDIR)/parser.o | $(QEMU-LDIR)
	$(ARX86) -rcs $@ $^ $(QEMU-BDIR)/parser.o

$(QEMU-BDIR)/%.o : $(QEMU-SRC)/%.c | $(QEMU-LDIR)
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
$(BDIR)/addsub.o: $(SRC)/addsub.c $(IDIR)/addsub.h
	$(CC) -c $< -o $@

$(BDIR)/muldiv.o: $(SRC)/muldiv.c $(IDIR)/muldiv.h
	$(CC) -c $< -o $@


###########   Parser   ###########
$(QEMU-BDIR)/parser.o: $(SRC)/parser.c $(IDIR)/parser.h | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(BDIR)/parser.o: $(SRC)/parser.c $(IDIR)/parser.h | $(LDIR)
	$(CC) -c $< -o $@

###########   Building Executeable   ###########
test: examples/test.c
	$(CC) -march=rv32imav -mabi=ilp32 $< $(cx_libraries) -L$(LDIR) -lci -O2 -o temp


###########   Running on different emulators   ###########
qemu: temp
	${RISCV}/riscv-gnu-toolchain/qemu/build/qemu-riscv32 -L ./utils/riscv/bin/ ./$^


### TODO: Modify spike to execute cx instructions
spike: temp
	${RISCV}/riscv-llvm/bin/spike --isa=rv32imav ${RISCV}/riscv-pk/build-llvm/pk $^


###########   Clean build   ###########
clean:
	rm -rf build/
	rm -rf build-qemu/
	rm -f temp