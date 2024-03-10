// qemu/target/riscv/
#include "qemu/osdep.h"
#include "exec/exec-all.h"
#include "exec/helper-proto.h"

#include <assert.h>

#include "../../../../../zoo/exports.h"

cx_func_stub_t cx_funcs[MAX_CX_ID] = {
    cx_func_addsub,
    cx_func_muldiv,
    cx_func_error // pad to MAX_CX_ID somehow ==> move to cx_init() ????
};

int32_t num_cfs[MAX_CX_ID] = {
    CX_ADDSUB_NUM_FUNCS,
    CX_MULDIV_NUM_FUNCS,
    CX_ERROR_NUM_FUNCS // pad to max ==> Should be moved somewhere else in qemu
};

target_ulong HELPER(cx_reg)(CPURISCVState *env, target_ulong cf_id, 
                             target_ulong rs1, target_ulong rs2)
{
    uint32_t OPCODE_ID = cf_id;
    int32_t OPA = rs1;
    int32_t OPB = rs2;
    int32_t CX_ID = GET_CX_ID(env->mcx_selector);
    
    assert( CX_ID < MAX_CX_ID); // Possibly redundant
    assert( OPCODE_ID <= num_cfs[CX_ID] );

    int32_t out = cx_funcs[CX_ID][OPCODE_ID](OPA, OPB);

    return (target_ulong)out;
} 