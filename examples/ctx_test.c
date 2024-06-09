#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "../../../../research/riscv-tools/cx_runtime/include/ci.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/mulacc/mulacc.h"

void state_test() {
  int a = 3;
  int b = 5;
  int result;

  cx_share_t share_A = 0, share_C = 0;

  int cx_sel_C0 = cx_open(CX_GUID_MULACC, share_C);
  assert(cx_sel_C0 == 1);

  printf("cx_sel_C0: %d\n", cx_sel_C0);
  printf("a: %d, b: %d\n", a, b);

  cx_sel(cx_sel_C0);
  result = mac(a, a);
  assert(result == 9);
  printf("result (mulacc: a * a, state_id: 0) : %d\n", result);
  
  uint cx_status = 1;
  CX_READ_STATUS(cx_status);
  printf("status: %08x\n", cx_status);

  cx_context_save();
  result = mac(a, b);
  // assert(result == 24);
  printf("result (mulacc: a * b, state_id: 0) : %d\n", result);
  
  cx_context_restore();
  CX_READ_STATE(result, 0);
  CX_READ_STATUS(cx_status);

  printf("result (should be 9, state_id: 0) : %d, status: %08x\n", result, cx_status);

  printf("done!\n");
  
  cx_sel(CX_LEGACY);


}

int main() {
    cx_init();
    cx_sel(CX_LEGACY);
    state_test();
    return 0;
}
