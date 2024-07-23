#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../../../research/riscv-tools/cx_runtime/include/ci.h"

#define CX_GUID_A 11
#define CX_GUID_B 4
// #define CX_GUID_C 222

int cx_reg(int a, int b) {
  CX_REG(0, a, b);
}

int main() {
    int a = 3;
    int b = 5;
    int result;

    cx_share_t share_A = 0, share_B = 0; //, share_C = 0;

    int cx_sel_A = cx_open(CX_GUID_A, share_A);
    int cx_sel_B = cx_open(CX_GUID_B, share_B);
    
    printf("cx_sel_A: %d, cx_sel_B: %d\n", cx_sel_A, cx_sel_B);

    cx_sel(cx_sel_B);
    result = cx_reg(a, b);
    printf("result (mul) : %d\n", result);

    cx_sel(cx_sel_A);
    result = cx_reg(a, b);
    printf("result (add) : %d\n", result);

    cx_close(cx_sel_A);
    cx_close(cx_sel_B);
    
    printf("completed!\n");
    return 0;
}
