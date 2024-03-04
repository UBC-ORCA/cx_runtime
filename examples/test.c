#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/ci.h"
#include "../include/addsub.h"
#include "../include/muldiv.h"

// definitions for cx_id's are in here
#include "../include/parser.h"

int main() 
{
    init_cfu_runtime();
    int32_t a = 3;
    int32_t b = 5;
    
    int32_t result = 0;

    cx_share_t cx_share_a = 0;
    cx_sel_t cx_sel_index_A = cx_open(CX_GUID_ADDSUB, cx_share_a);
    cx_sel_t cx_sel_index_B = cx_open(CX_GUID_MULDIV, cx_share_a);

    cx_sel_t prev_cx_sel_index = cx_select(cx_sel_index_A);

    // There should be something in the spec that defines what happens if 
    // this cx_select value is invalid e.g., greater than 1023
    

    // cx_get_curr_selection()
    // does csrr on cs_index
    
    // mcx_sel set to CX_GUID_ADDSUB
    result = add(a, b);
    printf("result add: %d\n", result);

    prev_cx_sel_index = cx_select(cx_sel_index_B);
    result = mul(a, b);
    printf("result mul: %d\n", result);

    prev_cx_sel_index = cx_select(cx_sel_index_A);

    result = sub(a, b);
    printf("result sub: %d\n", result);

    // mcx_sel set to CX_GUID_MULDIV

    cx_close(cx_sel_index_A);
    cx_close(cx_sel_index_B);
    
    return 0;
}