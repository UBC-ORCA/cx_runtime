.PHONY: clean all temp 

# CC = ${RISCV}/llvm/build-linux/bin/clang
CC = ${RISCV}riscv32-unknown-linux-gnu-gcc
AR = ${RISCV}riscv32-unknown-linux-gnu-ar

CCX86 = gcc
ARX86 = ar

BDIR := build
LDIR := $(BDIR)/lib
IDIR := include
SRC  := src
TEST := test

QEMU-BDIR := build-qemu
QEMU-LDIR := $(QEMU-BDIR)/lib
QEMU-IDIR := $(IDIR)/cx-qemu
QEMU-SRC := $(SRC)/cx-qemu

ZOO-DIR := zoo

cx_objects := $(BDIR)/ci.o $(BDIR)/queue.o $(BDIR)/parser.o
cx_libraries := $(BDIR)/addsub.o $(BDIR)/muldiv.o $(BDIR)/mulacc.o
cx_helpers := $(QEMU-BDIR)/addsub_func.o $(QEMU-BDIR)/muldiv_func.o $(QEMU-BDIR)/mulacc_func.o 
qemu_objects := $(cx_helpers) $(QEMU-BDIR)/exports.o

all: $(QEMU-LDIR)/libmcx_selector.so $(LDIR)/libci.a $(cx_libraries)


###########   Qemu functionality   ###########
$(QEMU-LDIR)/libmcx_selector.so: $(qemu_objects) | $(QEMU-LDIR)
	$(ARX86) -rcs $@ $^

$(QEMU-BDIR)/%.o : $(QEMU-SRC)/%.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-BDIR)/addsub_func.o : $(ZOO-DIR)/addsub/addsub_func.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-BDIR)/muldiv_func.o : $(ZOO-DIR)/muldiv/muldiv_func.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-BDIR)/mulacc_func.o : $(ZOO-DIR)/mulacc/mulacc_func.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-BDIR)/exports.o : $(ZOO-DIR)/exports.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-LDIR):
	mkdir -p $(QEMU-LDIR)


###########   Building cx runtime   ###########
$(LDIR)/libci.a: $(cx_objects) $(cx_libraries) | $(LDIR)
	$(AR) -rcs $@ $(cx_objects) $(cx_libraries)

$(BDIR)/%.o : $(SRC)/%.c | $(LDIR)
	$(CC) -c $< -o $@

$(LDIR):
	mkdir -p $(LDIR)


###########   CX Libraries   ###########
$(BDIR)/addsub.o: $(ZOO-DIR)/addsub/addsub.c $(ZOO-DIR)/addsub/addsub.h
	$(CC) -c $< -o $@

$(BDIR)/muldiv.o: $(ZOO-DIR)/muldiv/muldiv.c $(ZOO-DIR)/muldiv/muldiv.h
	$(CC) -c $< -o $@

$(BDIR)/mulacc.o: $(ZOO-DIR)/mulacc/mulacc.c $(ZOO-DIR)/mulacc/mulacc.h
	$(CC) -c $< -o $@


###########   Parser   ###########

$(BDIR)/parser.o: $(SRC)/parser.c $(IDIR)/parser.h | $(LDIR)
	$(CC) -c $< -o $@

###########   Building Executeable   ###########
example: examples/example.c $(LDIR)/libci.a
	$(CC) -march=rv32imav -mabi=ilp32 $< $(cx_libraries) -L$(LDIR) -lci -O2 -o example


###########   Running on different emulators   ###########
qemu: example
	${RISCV}/riscv-gnu-toolchain/qemu/build/riscv32-softmmu/qemu-system-riscv32 -nographic -machine virt \
	-kernel ~/Documents/linux_rv32/linux/arch/riscv/boot/Image \
	-initrd ~/Documents/linux_rv32/initramfs/initramfs.cpio.gz \
	-append "console=ttyS0"

### TODO: Modify spike to execute cx instructions
spike: example
	${RISCV}/riscv-gnu-toolchain/spike/build/spike --cxs=0,1 --isa=rv32imav --dump-dts ${RISCV}/riscv-gnu-toolchain/pk/build/pk $^
# ${RISCV}/riscv-gnu-toolchain/pk/build/pk $^


###########   tests   ###########
test: stateless_test stateful_test cx_table_test

stateless_test: $(TEST)/stateless_test.c $(LDIR)/libci.a
	$(CC) -march=rv32imav -mabi=ilp32 $< $(cx_libraries) -L$(LDIR) -lci -O2 -o $@

stateful_test: $(TEST)/stateful_test.c $(LDIR)/libci.a
	$(CC) -march=rv32imav -mabi=ilp32 $< $(cx_libraries) -L$(LDIR) -lci -O2 -o $@

cx_table_test: $(TEST)/cx_table_test.c $(LDIR)/libci.a
	$(CC) -march=rv32imav -mabi=ilp32 $< $(cx_libraries) -L$(LDIR) -lci -O2 -o $@

###########   Clean build   ###########
clean:
	rm -rf build/
	rm -rf build-qemu/
	rm -f example