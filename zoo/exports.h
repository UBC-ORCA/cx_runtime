#ifndef EXPORTS_H
#define EXPORTS_H

#include <stdint.h>

#include "addsub/addsub_func.h"
#include "muldiv/muldiv_func.h"
#include "mulacc/mulacc_func.h"

#define MAX_CX_ID 255
#define CX_ID_BITS 8
#define CX_STATE_ID_BITS 8
#define CX_ID_START_INDEX 0

#define CX_ERROR_NUM_FUNCS 0

#define GET_BITS(cx_sel, start_bit, n) \
    (cx_sel >> start_bit) & (((1 << n) - 1) )

#define GET_CX_ID(cx_sel) \
    GET_BITS(cx_sel, CX_ID_START_INDEX, CX_ID_BITS)

#define GET_CX_STATE(cx_sel) \
    GET_BITS(cx_sel, 16, CX_STATE_ID_BITS)

typedef int32_t (*(*cx_func_stub_t)) (int32_t, int32_t, int32_t);

extern int32_t (*cx_func_error[]) (int32_t, int32_t, int32_t);

extern cx_func_stub_t cx_funcs[MAX_CX_ID];

extern int32_t num_cfs[MAX_CX_ID];

#endif // EXPORTS_H