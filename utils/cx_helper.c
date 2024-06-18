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

    if (GET_CX_CXE(env->mcx_selector) == 1) {
        // RISCV_EXCP_ILLEGAL_INST
        // RISCV_EXCP_FIRST_CX_USE
        // RISCV_EXCP_U_ECALL
        // env->gpr[17] = 462;
        riscv_raise_exception(env, RISCV_EXCP_ILLEGAL_INST, GETPC());
    }

    uint32_t OPCODE_ID = cf_id;
    int32_t OPA = rs1;
    int32_t OPB = rs2;
    uint32_t CX_ID = GET_CX_ID(env->mcx_selector);
    uint32_t STATE_ID = GET_CX_STATE(env->mcx_selector);
    uint32_t VERSION = GET_CX_VERSION(env->mcx_selector);
    printf("helper cx_id: %d, a: %d, b: %d, mcx_selector: %08x\n", OPCODE_ID, OPA, OPB, env->mcx_selector);

    // not sure if these are the right error values to set it to
    if (env->mcx_selector == CX_INVALID_SELECTOR) {
        cx_status_t cx_status = {{env->cx_status}};
        cx_status.sel.IV = 1;
        cx_status.sel.IC = 1;
        cx_status.sel.IS = 1;
        env->cx_status = cx_status.idx;
        return (target_ulong)-1;
    }

    if (VERSION != 1) {
        cx_status_t cx_status = {{env->cx_status}};
        cx_status.sel.IV = 1;
        env->cx_status = cx_status.idx;
    }

    // stateless 
    if (STATE_ID > 0 && num_states[CX_ID] == 0) {
        cx_status_t cx_status = {{env->cx_status}};
        cx_status.sel.IS = 1;
        env->cx_status = cx_status.idx;
    }
    // stateful
    else if (STATE_ID > num_states[CX_ID] - 1) {
        cx_status_t cx_status = {{env->cx_status}};
        cx_status.sel.IS = 1;
        env->cx_status = cx_status.idx;
    }

    if (OPCODE_ID > num_cfs[CX_ID] - 1) {
        cx_status_t cx_status = {{env->cx_status}};
        cx_status.sel.IF = 1;
        env->cx_status = cx_status.idx;
    }
    
    assert( CX_ID < MAX_CX_ID); // Possibly redundant

    int32_t out = cx_funcs[CX_ID][OPCODE_ID](OPA, OPB, STATE_ID);

    return (target_ulong)out;
    // }
} 

// void HELPER(mcx_trap)(CPURISCVState *env)
// {
//     // uint mcx_selector = env->mcx_selector;
//     // int cxe = GET_CX_CXE(mcx_selector);
//     // printf("mcx_selector: %08x, cxe: %d\n", mcx_selector, cxe);
// 
//     // if (cxe == 1) {
//         // While there will be a csrw in the kernel, it will not update this selector value
//         // in time, as 2 instructions are executed in a single cx_reg instruction 
//         // (the cx_reg and this mcx_trap). There may be a better way to do this in qemu.
//         // int prev_val = env->gpr[17];
//         // env->gpr[17] = 462;
// 
//         // RISCV_EXCP_FIRST_CX_USE, RISCV_EXCP_U_ECALL
//         // env->mcx_selector &= ~(1 << (CX_CXE_START_INDEX));
//     riscv_raise_exception_cx(env, RISCV_EXCP_ILLEGAL_INST, GETPC());
//     // helper_raise_exception(env, RISCV_EXCP_ILLEGAL_INST );
//     printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
//         // env->gpr[17] = prev_val;
//     // }
// 
//     return;
// }