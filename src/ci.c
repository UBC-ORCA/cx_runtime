#include <stdint.h>
#include <stdlib.h>

#include "../include/ci.h"

// TODO: This should be called from somewhere in the kernel
void cx_init() {
  asm volatile (
    "li a7, 458;        \n\t"  // cx_init syscall
    "ecall;             \n\t"
  );
}

void cx_sel(int cx_index) {
  cx_csr_write(CX_INDEX, cx_index);
  // TODO (Brandon): This CX_REG_HELPER should not be here for running on hw. It's a hack
  // for running on qemu.
  CX_REG_HELPER(0, 0, 0);
}

int32_t cx_open(cx_guid_t cx_guid, cx_share_t cx_share) {

  register long cx_index asm("a0");
  register long a0 asm("a0") = cx_guid;
  register long a1 asm("a1") = cx_share;
  register long syscall_id asm("a7") = 457; // cx_open
  asm volatile ("ecall  # 0=%0   1=%1  2=%2  3=%3"
    : "=r"(cx_index)
    : "r"(a0), "r"(a1), "r"(syscall_id)
    :
  );
  return cx_index;
}

void cx_close(cx_sel_t cx_sel)
{
  int cx_close_error = -1;
  asm volatile (
    "li a7, 459;        \n\t"  // syscall 459, cx_open
    "mv a0, %0;         \n\t"  // a0-a5 are ecall args 
    "ecall;             \n\t"
    "mv %1, a0;         \n\t"
    :  "=r" (cx_close_error)
    :  "r"  (cx_sel)  
    : 
  );
}

cx_error_t cx_error_read() {
  return cx_csr_read(CX_STATUS);
}

void cx_error_clear() {
  cx_csr_write(CX_STATUS, 0);
}

void cx_deselect_and_close(cx_sel_t cx_sel)
{
    return;
}

void cx_context_save() {
  register long cx_index asm("a0");
  register long syscall_id asm("a7") = 460; // cx_context_save
  asm volatile ("ecall  # 0=%0"
    : 
    : "r"(syscall_id)
    :
  );
}

void cx_context_restore() {
  register long cx_index asm("a0");
  register long syscall_id asm("a7") = 461; // cx_context_restore
  asm volatile ("ecall  # 0=%0"
    : 
    : "r"(syscall_id)
    :
  );
}