// qemu/target/riscv/
#include "qemu/osdep.h"
#include "exec/exec-all.h"
#include "exec/helper-proto.h"

#include <assert.h>
#include <stdio.h>

#include "../../../../../zoo/exports.h"


target_ulong HELPER(cx_reg)(CPURISCVState *env, target_ulong cf_id, 
                             target_ulong rs1, target_ulong rs2)
{
    uint32_t OPCODE_ID = cf_id;
    int32_t OPA = rs1;
    int32_t OPB = rs2;
    int32_t CX_ID = GET_CX_ID(env->mcx_selector);
    int32_t STATE_ID = GET_CX_STATE(env->mcx_selector);
    // printf("STATE_ID from cx_helper: %08x\n", STATE_ID);
    // printf("MCX_SELECTOR: %08x\n", env->mcx_selector);
    
    assert( CX_ID < MAX_CX_ID); // Possibly redundant
    // assert( OPCODE_ID <= num_cfs[CX_ID] );
    // printf("CX_ID: %d, OPCODE_ID: %d\n", CX_ID, OPCODE_ID);

    int32_t out = cx_funcs[CX_ID][OPCODE_ID](OPA, OPB, STATE_ID);

    // int32_t out = OPA + OPB;
    return (target_ulong)out;
} 