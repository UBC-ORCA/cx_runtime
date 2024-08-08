.PHONY: clean machine

# CC = ${RISCV}/llvm/build-linux/bin/clang
CC = riscv32-unknown-elf-gcc
AR = riscv32-unknown-elf-ar

CCX86 = gcc
ARX86 = ar

BDIR := build
LDIR := $(BDIR)/lib
IDIR := include
SRC  := src
TEST := tests

ZOO-DIR := zoo

cx_objects_m := $(BDIR)/ci_m.o

machine: $(LDIR)/libci_m.a

$(QEMU-LDIR):
	mkdir -p $(QEMU-LDIR)

###########   Building cx runtime   ###########
$(LDIR)/libci_m.a: $(cx_objects_m) | $(LDIR)
	$(AR) -rcs $@ $(cx_objects_m)

$(BDIR)/%.o : $(SRC)/%.c | $(LDIR)
	$(CC) -c $< -o $@

$(LDIR):
	mkdir -p $(LDIR)

###########   tests   ###########

state_test_m: $(TEST)/state_test_m.c
	$(CC) -static $< -o state_test_m -L $(LDIR) -lci_m

###########   Building Executeable   ###########
example_m: examples/example.c
	$(CC) -static $< -o example_m -L $(LDIR) -lci_m

###########   Clean build   ###########
clean:
	rm -rf build/
	rm -rf build-qemu/
	rm -f example