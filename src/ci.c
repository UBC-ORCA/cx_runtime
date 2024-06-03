#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/ci.h"

// TODO: This should be called from somewhere in the kernel
void cx_init() {
  asm volatile (
    "li a7, 458;        \n\t"  // cx_init syscall
    "ecall;             \n\t"
  );
}

void cx_sel(int cx_index) {
   asm volatile (
    "csrw " CX_INDEX ", %0        \n\t" // TODO: Should be 800
    :
    : "r" (cx_index)
    :
   );
}

int32_t cx_open(cx_guid_t cx_guid, cx_share_t cx_share) {
  int cx_index = -1;
  asm volatile (
    "li a7, 457;        \n\t"  // syscall 51, cx_open
    "mv a0, %0;         \n\t"  // a0-a5 are ecall args 
    "ecall;             \n\t"
    "mv %1, a0;         \n\t" 
    :  "=r" (cx_index)
    :  "r"  (cx_guid)
    :
  );
  return cx_index;
}

void cx_close(cx_sel_t cx_sel)
{
  int cx_close_error = -1;
  asm volatile (
    "li a7, 459;        \n\t"  // syscall 51, cx_open
    "mv a0, %0;         \n\t"  // a0-a5 are ecall args 
    "ecall;             \n\t"
    "mv %1, a0;         \n\t" 
    :  "=r" (cx_close_error)
    :  "r"  (cx_sel)
    : 
  );
}

uint cx_error() {
  int cx_error = -1;
  asm volatile (
    "csrr %0, " CX_STATUS ";     \n\t"
    : "=r" (cx_error)
    :
    :
  );
  if (cx_error == -1) {
    printf("error reading cx_error\n");
    exit ( -1 );
  }
  return cx_error;
}


void cx_deselect_and_close(cx_sel_t cx_sel)
{
    return;
}
