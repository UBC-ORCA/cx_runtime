#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/ci.h"
#include "../include/addsub.h"

int main() 
{
    init_cfu_runtime();
    int32_t a = 3;
    int32_t b = 5;
    
    int32_t result = 0;

    cx_share_t cx_share_a = 0;
    cx_sel_t cx_sel_index_A = cx_open(CX_GUID_ADDSUB, cx_share_a);
    cx_sel_t cx_sel_index_B = cx_open(CX_GUID_B, cx_share_a);

    cx_sel_t prev_cx_sel_index = cx_select(cx_sel_index_A);
    
    // mcx_sel set to CX_GUID_ADDSUB
    result = add(a, b);
    printf("result add: %d\n", result);
    result = sub(result, b);
    printf("result sub: %d\n", result);

    prev_cx_sel_index = cx_select(cx_sel_index_B);

    cx_close(cx_sel_index_A);
    cx_close(cx_sel_index_B);
    
    return 0;
}