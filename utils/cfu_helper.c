// qemu/target/riscv/
#include "qemu/osdep.h"
#include "exec/exec-all.h"
#include "exec/helper-proto.h"

#include <stdio.h>
#include "../../../../../zoo/addsub/addsub_func.h"
#include "../../../../../zoo/muldiv/muldiv_func.h"

static int (*(*cx_funcs[]))(int, int) = {
    addsub_func, muldiv_func
};

target_ulong HELPER(cfu_reg)(CPURISCVState *env, target_ulong cf_id, 
                             target_ulong rs1, target_ulong rs2)
{
    uint32_t CF_ID = cf_id;
    int32_t RS1 = rs1;
    int32_t RS2 = rs2;
    int32_t out = cx_funcs[env->mcfu_selector][CF_ID](RS1, RS2);
    
    return out;
} 
