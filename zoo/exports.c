#include "exports.h"

// structs populated by spike / qemu

int32_t (*cx_func_error[]) (int32_t, int32_t, int32_t) = {0};

cx_func_stub_t cx_funcs[MAX_CX_ID] = {
    cx_func_addsub,
    cx_func_muldiv,
    cx_func_mulacc
    };

int32_t num_cfs[MAX_CX_ID] = {
    CX_ADDSUB_NUM_FUNCS,
    CX_MULDIV_NUM_FUNCS, 
    CX_MULACC_NUM_FUNCS
    };
