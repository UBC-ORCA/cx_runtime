#include <stdint.h>

#include "../../include/cx-qemu/addsub_func.h"

static inline int32_t add_func(int32_t a, int32_t b)
{
    return a + b;
}

static inline int32_t sub_func(int32_t a, int32_t b)
{
    return a - b;
}

int32_t addsub_sel(int32_t cf_id, int32_t rs1, int32_t rs2) 
{
    if (cf_id == 0) {
        return add_func(rs1, rs2);
    } else if (cf_id == 1) {
        return sub_func(rs1, rs2);
    }
    return 0;
}