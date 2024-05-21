#include <stdint.h>

#include "mulacc_func.h"

static int acc[CX_MULACC_NUM_STATES];

static inline int32_t mac_func(int32_t a, int32_t b, int32_t state_id)
{
    acc[state_id] += a * b;
    return acc[state_id];
}

static inline int32_t reset_func(int32_t a, int32_t b, int32_t state_id)
{
    acc[state_id] = 0;
    return 0;
}

int32_t (*cx_func_mulacc[]) (int32_t, int32_t, int32_t) = {
    mac_func,
    reset_func
};
// FIXME: need to make sure the right number of functions are defined
// and agree with code in cfu_helper.c:num_opcodes_for_this_CX_ID
