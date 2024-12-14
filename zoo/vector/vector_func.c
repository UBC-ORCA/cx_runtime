#include <stdint.h>

#include "vector_func.h"
#include "vector_common.h"
#include "../../include/utils.h"

static uint regs[CX_VECTOR_NUM_STATES][CX_VECTOR_NUM_REGS][CX_VECTOR_VL];

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


static cx_stctxs_t cxu_stctx_status[CX_VECTOR_NUM_STATES];

static inline int32_t add_func(int32_t a, int32_t b, int32_t state_id)
{
    cxu_stctx_status[state_id].sel.dc = CX_DIRTY;
    for (int i = 0; i < CX_VECTOR_VL; i++) {
        regs[state_id][a][i] += regs[state_id][b][i];
    }
    return 0;
}

static inline int32_t set_inc_func(int32_t a, 
                                   __attribute__((unused)) int32_t b, 
                                   int32_t state_id)
{
    cxu_stctx_status[state_id].sel.dc = CX_DIRTY;
    for (int i = 0; i < CX_VECTOR_VL; i++) {
        regs[state_id][a][i] = i;
    }

    return 0;
}

static inline int32_t set2_func(int32_t a, 
                                __attribute__((unused)) int32_t b, 
                                int32_t state_id)
{
    cxu_stctx_status[state_id].sel.dc = CX_DIRTY;
    for (int i = 0; i < CX_VECTOR_VL; i++) {
        regs[state_id][a][i] = 2;
    }
    return 0;
}

static inline int32_t reset_func(__attribute__((unused)) int32_t a, 
                                 __attribute__((unused)) int32_t b, 
                                 int32_t state_id) {
    for (int k = 0; k < CX_VECTOR_NUM_REGS; k++) {
        for (int j = 0; j < CX_VECTOR_VL; j++) {
            regs[state_id][k][j];
        }
    }
    return 0;
}

static inline int32_t vector_read_status_func( __attribute__((unused)) int32_t unused0, 
                                               __attribute__((unused)) int32_t unused1, 
                                               int32_t state_id ) {
    return cxu_stctx_status[state_id].idx;
};

static inline int32_t vector_write_status_func( int32_t value, 
                                                __attribute__((unused)) int32_t unused0,
                                                int32_t state_id ) {    
    uint cx_status = GET_CX_DATA_CLEAN(value);

    if (cx_status == CX_OFF) {
        cxu_stctx_status[state_id] = off_status_word;
    } else if (cx_status == CX_PRECLEAN) {
        // Write initial first, in case state is read. SW will know that CXU is still
        // in the process of resetting.
        cxu_stctx_status[state_id] = initial_status_word;
        // hw update to reset state.
        for (int i = 0; i < CX_VECTOR_NUM_REGS; i++) {
            reset_func(i, 0, state_id);
        }
        // write CX_DIRTY, so OS knows to save state on context switch
        cxu_stctx_status[state_id].sel.dc = CX_DIRTY;
    }
    else if (cx_status == CX_DIRTY)
    {
        cxu_stctx_status[state_id].sel.dc = CX_DIRTY;
    }
    else if (cx_status == CX_CLEAN)
    {
        cxu_stctx_status[state_id].sel.dc = CX_CLEAN;
    }

    return 0;
};

static inline int32_t vector_read_state_func( int32_t index,
                                              __attribute__((unused)) int32_t unused0, 
                                              int32_t state_id ) {
    if (index >= CX_VECTOR_STATE_SIZE) {
        return 0xFFFFFFFF;
    }
    uint reg = (index / CX_VECTOR_VL) % 4;
    uint idx = index % CX_VECTOR_VL;
    return regs[state_id][reg][idx];
}

static inline int32_t vector_write_state_func( int32_t index, 
                                               int32_t value, 
                                               int32_t state_id ) {
    if (index >= CX_VECTOR_STATE_SIZE) {
        return 0xFFFFFFFF;
    }

    uint reg = (index / CX_VECTOR_VL) % 4;
    uint idx = index % CX_VECTOR_VL;
    return regs[state_id][reg][idx] = value;
};

int32_t (*cx_func_vector[MAX_CF_IDS]) (int32_t, int32_t, int32_t) = {
    add_func,
    set_inc_func,
    set2_func,
    reset_func
};

// TODO: This should be moved to another file
static int32_t cx_func_undefined (int32_t, int32_t, int32_t) { return -1; }

void init_cx_func_vector() {
    for (int i = CX_VECTOR_NUM_FUNCS; i < MAX_CF_IDS - 4; i++) {
        cx_func_vector[i] = cx_func_undefined;
    }

    for (int i = 0; i < CX_VECTOR_NUM_STATES; i++) {
        cxu_stctx_status[i] = off_status_word;
    }

    cx_func_vector[1020] = vector_write_state_func;
    cx_func_vector[1021] = vector_read_state_func;
    cx_func_vector[1022] = vector_write_status_func;
    cx_func_vector[1023] = vector_read_status_func;
}