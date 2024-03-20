#include <stdio.h>
#include <assert.h>

uint32_t OPCODE_ID = insn.cf_id();
int32_t OPA = RS1;
int32_t OPB = RS2;

uint32_t CX_ID = GET_CX_ID(STATE.mcx_selector->read());

assert( CX_ID < MAX_CX_ID); // Possibly redundant
assert( OPCODE_ID <= num_cfs[CX_ID] );

WRITE_RD(cx_funcs[CX_ID][OPCODE_ID](OPA, OPB));