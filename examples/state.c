#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../../../research/riscv-tools/cx_runtime/include/ci.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/mulacc/mulacc.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/addsub/addsub.h"

void cx_error_test( cx_sel_t cx_sel ) {
    cx_error_t cx_error = cx_error_read();
    if (cx_error > 0) {
        fprintf( stderr, "error: cx_error %08x with cx_sel %d\n", cx_error, cx_sel );
        exit( -1 );
    }
}

void state_test() {
  int a = 3;
  int b = 5;
  int result;

  cx_share_t share_A = 0, share_C = 0;

  int cx_sel_C0 = cx_open(CX_GUID_MULACC, share_C);
  int cx_sel_C1 = cx_open(CX_GUID_MULACC, share_C);

  printf("cx_sel_C0: %d, cx_sel_C: %d\n", cx_sel_C0, cx_sel_C1);
  printf("a: %d, b: %d\n", a, b);

  cx_error_clear();
  cx_sel(cx_sel_C0);
  result = mac(a, a);
  cx_error_test(cx_sel_C0);
  printf("result (mulacc: a * a, state_id: 0) : %d\n", result);

  cx_error_clear();
  cx_sel(cx_sel_C1);
  result = mac(b, b);
  cx_error_test(cx_sel_C1);
  printf("result (mulacc: b * b, state_id: 1) : %d\n", result);

  cx_error_clear();
  cx_sel(cx_sel_C0);
  result = mac(a, a);
  cx_error_test(cx_sel_C0);
  printf("result (mulacc: a * a, state_id: 0) : %d\n", result);

  cx_error_clear();
  cx_sel(cx_sel_C1);
  result = mac(a, b);
  cx_error_test(cx_sel_C1);
  printf("result (mulacc: a * b, state_id: 1) : %d\n", result);

  cx_close(cx_sel_C0);
  cx_close(cx_sel_C1);
  cx_sel( CX_LEGACY );
}

int main() {
    cx_sel( CX_LEGACY );
    state_test();
    return 0;
}
