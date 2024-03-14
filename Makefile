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
TEST := test

QEMU-BDIR := build-qemu
QEMU-LDIR := $(QEMU-BDIR)/lib
QEMU-IDIR := $(IDIR)/cx-qemu
QEMU-SRC := $(SRC)/cx-qemu

ZOO-DIR := zoo


cx_objects := $(BDIR)/ci.o $(BDIR)/queue.o $(BDIR)/parser.o
cx_libraries := $(BDIR)/addsub.o $(BDIR)/muldiv.o
cx_helpers := $(QEMU-BDIR)/addsub_func.o $(QEMU-BDIR)/muldiv_func.o
qemu_objects := $(cx_helpers)

all: $(QEMU-LDIR)/libmcx_selector.so $(LDIR)/libci.so $(cx_libraries)


###########   Qemu functionality   ###########
$(QEMU-LDIR)/libmcx_selector.so: $(qemu_objects) | $(QEMU-LDIR)
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
example: examples/example.c $(LDIR)/libci.so
	$(CC) -march=rv32imav -mabi=ilp32 $< $(cx_libraries) -L$(LDIR) -lci -O2 -o example


###########   Running on different emulators   ###########
qemu: cx_table_test
	${RISCV}/riscv-gnu-toolchain/qemu/build/qemu-riscv32 -L ./utils/riscv/bin/ ./$^
# -virtfs local,path=/home/bf/research/riscv-tools/cx_runtime/,mount_tag=host0,security_model=passthrough,id=host0

### TODO: Modify spike to execute cx instructions
spike: example
	${RISCV}/riscv-llvm/bin/spike --isa=rv32imav ${RISCV}/riscv-pk/build-llvm/pk $^



###########   tests   ###########
test: stateless_test stateful_test cx_table_test

stateless_test: $(TEST)/stateless_test.c $(LDIR)/libci.so
	$(CC) -march=rv32imav -mabi=ilp32 $< $(cx_libraries) -L$(LDIR) -lci -O2 -o $@

stateful_test: $(TEST)/stateful_test.c $(LDIR)/libci.so
	$(CC) -march=rv32imav -mabi=ilp32 $< $(cx_libraries) -L$(LDIR) -lci -O2 -o $@

cx_table_test: $(TEST)/cx_table_test.c $(LDIR)/libci.so
	$(CC) -march=rv32imav -mabi=ilp32 $< $(cx_libraries) -L$(LDIR) -lci -O2 -o $@

###########   Clean build   ###########
clean:
	rm -rf build/
	rm -rf build-qemu/
	rm -f example