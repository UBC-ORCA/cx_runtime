#include <stdint.h>

#include "addsub_func.h"

static inline int32_t add_func(int32_t a, int32_t b, int32_t state_id)
{
    return a + b;
}

static inline int32_t sub_func(int32_t a, int32_t b, int32_t state_id)
{
    return a - b;
}

static inline int32_t add_1000(int32_t a, int32_t b, int32_t state_id)
{
    return a + b + 1000;
}

int32_t (*cx_func_addsub[]) (int32_t, int32_t, int32_t) = {
    add_func,
    sub_func,
    add_1000
};
// FIXME: need to make sure the right number of functions are defined
// and agree with code in cfu_helper.c:num_opcodes_for_this_CX_ID