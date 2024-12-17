#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "vector_func.h"
#include "vector_common.h"
#include "../../include/utils.h"

static uint regs[CX_NUM_VEC_INSTANCES][CX_VECTOR_NUM_STATES][CX_VECTOR_NUM_REGS][CX_VECTOR_VL];

static const cx_stctxs_t initial_status_word = {.sel = {.dc = CX_PRECLEAN,
                                                        .state_size = CX_VECTOR_STATE_SIZE,
                                                        .reserved0 = 0,
                                                        .version = 1,
                                                        .R = 0}};

static const cx_stctxs_t off_status_word = {.sel = {.dc = CX_OFF,
                                                        .state_size = CX_VECTOR_STATE_SIZE,
                                                        .reserved0 = 0,
                                                        .version = 1,
                                                        .R = 0}};

static cx_stctxs_t cxu_stctx_status[CX_NUM_VEC_INSTANCES][CX_VECTOR_NUM_STATES];

static int cxu_id_to_instance_num(int cx_id) {
    switch (cx_id)
    {
    case 4:
        return 0;
    case 5:
        return 1;
    default:
        printf("invalid cx_id in mulacc unit\n");
        exit(1);
    }
    return 0;
}

static inline int32_t add_func(int32_t a, int32_t b, cx_selidx_t sys_sel)
{
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    cxu_stctx_status[inst_num][state_id].sel.dc = CX_DIRTY;
    for (int i = 0; i < CX_VECTOR_VL; i++) {
        regs[inst_num][state_id][a][i] += regs[inst_num][state_id][b][i];
    }
    return 0;
}

static inline int32_t set_inc_func(int32_t a, 
                                   __attribute__((unused)) int32_t b, 
                                   cx_selidx_t sys_sel)
{
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    cxu_stctx_status[inst_num][state_id].sel.dc = CX_DIRTY;
    for (int i = 0; i < CX_VECTOR_VL; i++) {
        regs[inst_num][state_id][a][i] = i;
    }

    return 0;
}

static inline int32_t set2_func(int32_t a, 
                                __attribute__((unused)) int32_t b, 
                                cx_selidx_t sys_sel)
{
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    cxu_stctx_status[inst_num][state_id].sel.dc = CX_DIRTY;
    for (int i = 0; i < CX_VECTOR_VL; i++) {
        regs[inst_num][state_id][a][i] = 2;
    }
    return 0;
}

static inline int32_t reset_func(__attribute__((unused)) int32_t a, 
                                 __attribute__((unused)) int32_t b, 
                                 cx_selidx_t sys_sel) {
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    for (int k = 0; k < CX_VECTOR_NUM_REGS; k++) {
        for (int j = 0; j < CX_VECTOR_VL; j++) {
            regs[inst_num][state_id][k][j];
        }
    }
    return 0;
}

static inline int32_t vector_read_status_func( __attribute__((unused)) int32_t unused0, 
                                               __attribute__((unused)) int32_t unused1, 
                                               cx_selidx_t sys_sel ) {
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    return cxu_stctx_status[inst_num][state_id].idx;
};

static inline int32_t vector_write_status_func( int32_t value, 
                                                __attribute__((unused)) int32_t unused0,
                                                cx_selidx_t sys_sel ) {    
    uint cx_status = GET_CX_DATA_CLEAN(value);
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    if (cx_status == CX_OFF) {
        cxu_stctx_status[inst_num][state_id] = off_status_word;
    } else if (cx_status == CX_PRECLEAN) {
        // Write initial first, in case state is read. SW will know that CXU is still
        // in the process of resetting.
        cxu_stctx_status[inst_num][state_id] = initial_status_word;
        // hw update to reset state.
        for (int i = 0; i < CX_VECTOR_NUM_REGS; i++) {
            reset_func(i, 0, sys_sel);
        }
        // write CX_DIRTY, so OS knows to save state on context switch
        cxu_stctx_status[inst_num][state_id].sel.dc = CX_DIRTY;
    }
    else if (cx_status == CX_DIRTY)
    {
        cxu_stctx_status[inst_num][state_id].sel.dc = CX_DIRTY;
    }
    else if (cx_status == CX_CLEAN)
    {
        cxu_stctx_status[inst_num][state_id].sel.dc = CX_CLEAN;
    }

    return 0;
};

static inline int32_t vector_read_state_func( int32_t index,
                                              __attribute__((unused)) int32_t unused0, 
                                              cx_selidx_t sys_sel ) {
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    if (index >= CX_VECTOR_STATE_SIZE) {
        return 0xFFFFFFFF;
    }
    uint reg = (index / CX_VECTOR_VL) % 4;
    uint idx = index % CX_VECTOR_VL;
    return regs[inst_num][state_id][reg][idx];
}

static inline int32_t vector_write_state_func( int32_t index, 
                                               int32_t value, 
                                               cx_selidx_t sys_sel ) {
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    if (index >= CX_VECTOR_STATE_SIZE) {
        return 0xFFFFFFFF;
    }

    uint reg = (index / CX_VECTOR_VL) % 4;
    uint idx = index % CX_VECTOR_VL;
    return regs[inst_num][state_id][reg][idx] = value;
};

int32_t (*cx_func_vector[MAX_CF_IDS]) (int32_t, int32_t, cx_selidx_t) = {
    add_func,
    set_inc_func,
    set2_func,
    reset_func
};

// TODO: This should be moved to another file
static int32_t cx_func_undefined (int32_t, int32_t, cx_selidx_t) { return -1; }

void init_cx_func_vector() {
    for (int i = CX_VECTOR_NUM_FUNCS; i < MAX_CF_IDS - 4; i++) {
        cx_func_vector[i] = cx_func_undefined;
    }

    for (int j = 0; j < CX_NUM_VEC_INSTANCES; j++) {
        for (int i = 0; i < CX_VECTOR_NUM_STATES; i++) {
            cxu_stctx_status[j][i] = off_status_word;
        }
    }

    cx_func_vector[1020] = vector_write_state_func;
    cx_func_vector[1021] = vector_read_state_func;
    cx_func_vector[1022] = vector_write_status_func;
    cx_func_vector[1023] = vector_read_status_func;
}