// qemu/target/riscv/
#include "qemu/osdep.h"
#include "exec/exec-all.h"
#include "exec/helper-proto.h"

//#include <stdio.h>
#include <assert.h>

#include "../../../../../zoo/exports.h"

typedef int (*(*cx_funcs))(int, int)  cx_func_stub_t;

extern cx_func_stub_t[MAX_CX_ID] = {
    cx_func_addsubcxu,
    cx_func_muldivcxu,
    cx_func_error // pad to MAX_CX_ID somehow ==> move to cx_init() ????

};

target_ulong HELPER(cfu_reg)(CPURISCVState *env, target_ulong cf_id, 
                             target_ulong rs1, target_ulong rs2)
{
    uint32_t OPCODE_ID = cf_id;
    int32_t OPA = rs1;
    int32_t OPB = rs2;
    assert( OPCODE_ID <= num_opcodes_for_this_CX_ID ); // defensive programming
    int32_t out = cx_funcs[GET_CX_ID(env->mcx_selector)][OPCODE_ID](OPA, OPB);

    return (target_ulong)out;
} 