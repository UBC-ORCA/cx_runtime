#include "exports.h"

// structs populated by spike / qemu

int32_t (*cx_func_error[]) (int32_t, int32_t, int32_t) = {0};

cx_func_stub_t cx_funcs[MAX_CX_ID] = {
    cx_func_muldiv,
    cx_func_addsub,
    cx_func_mulacc
    };

int32_t num_cfs[MAX_CX_ID] = {
    CX_MULDIV_NUM_FUNCS, 
    CX_ADDSUB_NUM_FUNCS,
    CX_MULACC_NUM_FUNCS
    };

// Fill unused functions in their arrays error
void cx_init_funcs() {
    init_cx_func_mulacc();

    for (int i = NUM_CX; i < MAX_CX_ID; i++) {
        cx_funcs[i] = cx_func_error;
        num_cfs[i] = 0;
    }
}