#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../../../research/riscv-tools/cx_runtime/include/ci.h"

#define CX_GUID_A 11
#define CX_GUID_C 222

int cx_reg(int a, int b) {
  CX_REG(0, a, b);
}

int main() {
    cx_init();
    int a = 3;
    int b = 5;
    int result;

    cx_share_t share_A = 0, share_C = 0;

    int cx_sel_A = cx_open(CX_GUID_A, share_A);
    int cx_sel_C0 = cx_open(CX_GUID_C, share_C);
    int cx_sel_C1 = cx_open(CX_GUID_C, share_C);

    printf("cx_sel_A: %d, cx_sel_C0: %d, cx_sel_C: %d\n", cx_sel_A, cx_sel_C0, cx_sel_C1);
    printf("a: %d, b: %d\n", a, b);

    cx_sel(cx_sel_C0);
    result = cx_reg(a, a);
    printf("result (mulacc: a * a, state_id: 0) : %d\n", result);

    cx_sel(cx_sel_C1);
    result = cx_reg(b, b);
    printf("result (mulacc: b * b, state_id: 1) : %d\n", result);

    cx_sel(cx_sel_A);
    result = cx_reg(a, b);
    printf("result (add: a + b) : %d\n", result);

    cx_sel(cx_sel_C0);
    result = cx_reg(a, a);
    printf("result (mulacc: a * a, state_id: 0) : %d\n", result);

    cx_sel(cx_sel_C1);
    result = cx_reg(a, b);
    printf("result (mulacc: a * b, state_id: 1) : %d\n", result);

    cx_close(cx_sel_A);
    cx_close(cx_sel_C0);
    cx_close(cx_sel_C1);

    
    printf("completed!\n");
    return 0;
}
