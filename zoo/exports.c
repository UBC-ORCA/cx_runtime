#include "exports.h"

// structs populated by spike / qemu

int32_t (*cx_func_error[]) (int32_t, int32_t) = {0};

cx_func_stub_t cx_funcs[MAX_CX_ID] = {
    cx_func_addsub,
    cx_func_muldiv};

int32_t num_cfs[MAX_CX_ID] = {10, 10, 10};