#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
// #include <stdbool.h>

#include "../include/ci.h"

#define MAX_CXU_ID 1 << CX_ID_BITS
#define MAX_STATE_ID 1 << STATE_ID_BITS

#define CX_SEL_TABLE_NUM_ENTRIES 1024
#define VERSION_START_INDEX 28
#define DEBUG 1
#define UNASSIGNED_STATE -1

#define MCX_SELECTOR 0xBC0
#define CX_STATUS    0x801
#define MCX_TABLE    0x802 // should be 0xBC1
#define CX_INDEX     0x800

#define MCX_VERSION 1

// TODO: This should be called from somewhere in the kernel
void cx_init() {
  asm volatile (
    "li a7, 458;        \n\t"  // cx_init syscall
    "ecall;             \n\t"
  );
}

int cx_sel(int cx_index) {
   asm volatile (
    "csrw 0x011, %0        \n\t" // TODO: Should be 800
    :
    : "r" (cx_index)
    :
   );
   return 0;
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

  if (cx_close_error == -1) {
    printf("error: with cx_close\n");
    exit(0);
  }
}

void cx_deselect_and_close(cx_sel_t cx_sel)
{
    return;
}
