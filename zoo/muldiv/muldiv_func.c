#include <stdint.h>

#include "muldiv_func.h"

static inline int32_t mul_func(int32_t a, int32_t b, int32_t state_id)
{
    return a * b;
}

static inline int32_t div_func(int32_t a, int32_t b, int32_t state_id)
{
    return a / b;
}

int32_t (*cx_func_muldiv[]) (int32_t, int32_t, int32_t) = {
    mul_func,
    div_func
};
// FIXME: need to make sure the right number of functions are defined
// and agree with code in cfu_helper.c:num_opcodes_for_this_CX_ID