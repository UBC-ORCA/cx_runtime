#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "ci.h"

int main() 
{
    init_cfu_runtime();
    int32_t a = 3;
    int32_t b = 5;
    int32_t cf_id = 2;

    cx_share_t cx_share_a = 0;
    cx_sel_t cx_sel_index_A = cx_open(CX_GUID_A, cx_share_a);
    cx_sel_t cx_sel_index_B = cx_open(CX_GUID_B, cx_share_a);

    cx_sel_t prev_cx_sel_index = cx_select(cx_sel_index_A);
    prev_cx_sel_index = cx_select(cx_sel_index_B);

    cx_close(cx_sel_index_A);
    cx_close(cx_sel_index_B);

    return 0;
}