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

  cx_sel(cx_sel_C0);
  result = mac(a, a);
  assert(result == 9);  

  cx_stctxs_t expected_stctxs = {.sel = {
                                    .cs = DIRTY,
                                    .error = 0,
                                    .initializer = CX_HW_INIT,
                                    .state_size = 1
                                  }};

  uint cx_status = CX_READ_STATUS();
  assert( cx_status == expected_stctxs.idx );

  cx_context_save();
  result = mac(a, b);
  assert( result == 15 );
  int32_t state_result = CX_READ_STATE(0);
  assert(result == state_result);
  
  cx_context_restore();
  result = CX_READ_STATE(0);
  cx_status = CX_READ_STATUS();
  expected_stctxs.sel.cs = CLEAN;
  assert( cx_status == expected_stctxs.idx );
  
  cx_sel(CX_LEGACY);
}

int main() {
    cx_init();
    cx_sel(CX_LEGACY);
    state_test();
    printf("Context save / restore test complete\n");

    return 0;
}
