#include <stdint.h>

#include "mulacc_func.h"
#include "../../include/utils.h"

static int acc[CX_MULACC_NUM_STATES];

static const cx_stctxs_t initial_status_word = {.sel = {.cs = CX_INITIAL,
                                                        .initializer = CX_HW_INIT,
                                                        .state_size = 1,
                                                        .reserved0 = 0,
                                                        .error = 0}};

static const cx_stctxs_t off_status_word = {.sel = {.cs = CX_OFF,
                                                        .initializer = CX_HW_INIT,
                                                        .state_size = 1,
                                                        .reserved0 = 0,
                                                        .error = 0}};


static cx_stctxs_t cxu_stctx_status[CX_MULACC_NUM_STATES];

static inline int32_t mac_func(int32_t a, int32_t b, int32_t state_id)
{
    cxu_stctx_status[state_id].sel.cs = CX_DIRTY;
    acc[state_id] += a * b;
    return acc[state_id];
}

static inline int32_t reset_func(int32_t a, int32_t b, int32_t state_id)
{
    acc[state_id] = 0;
    return 0;
}

static inline int32_t mulacc_read_status_func( __attribute__((unused)) int32_t unused0, 
                                               __attribute__((unused)) int32_t unused1, 
                                               int32_t state_id ) {
    return cxu_stctx_status[state_id].idx;
};

static inline int32_t mulacc_write_status_func( int32_t value, 
                                                __attribute__((unused)) int32_t unused0,
                                                int32_t state_id ) {    
    uint cx_status = GET_CX_STATUS(value);

    if (cx_status == CX_OFF) {
        cxu_stctx_status[state_id] = off_status_word;
    } else if (cx_status == CX_INITIAL) {
        // Write initial first, in case state is read. SW will know that CXU is still
        // in the process of resetting.
        cxu_stctx_status[state_id] = initial_status_word;
        // hw update to reset state.
        reset_func(0, 0, state_id);
        // write CX_DIRTY, so OS knows to save state on context switch
        cxu_stctx_status[state_id].sel.cs = CX_DIRTY;
    }
    else if (cx_status == CX_DIRTY)
    {
        cxu_stctx_status[state_id].sel.cs = CX_DIRTY;
    }
    else if (cx_status == CX_CLEAN)
    {
        cxu_stctx_status[state_id].sel.cs = CX_CLEAN;
    }

    return 0;
};

static inline int32_t mulacc_read_state_func( __attribute__((unused)) int32_t unused0, int32_t index, int32_t state_id ) {
    switch (index) {
        case 0:
            return acc[state_id];
            break;
        default: 
            return 0xFFFFFFFF;
    };
}

static inline int32_t mulacc_write_state_func( int32_t index, int32_t value, int32_t state_id ) {
    switch (index) {
        case 0:
            acc[state_id] = value;
            break;
        default: 
            return 0xFFFFFFFF;
    };
};

int32_t (*cx_func_mulacc[MAX_CF_IDS]) (int32_t, int32_t, int32_t) = {
    mac_func,
    reset_func
};

// TODO: This should be moved to another file
static int32_t cx_func_undefined (int32_t, int32_t, int32_t) { return -1; }

void init_cx_func_mulacc() {
    for (int i = CX_MULACC_NUM_FUNCS; i < MAX_CF_IDS - 4; i++) {
        cx_func_mulacc[i] = cx_func_undefined;
    }

    for (int i = 0; i < CX_MULACC_NUM_STATES; i++) {
        cxu_stctx_status[i] = off_status_word;
    }

    cx_func_mulacc[1020] = mulacc_write_state_func;
    cx_func_mulacc[1021] = mulacc_read_state_func;
    cx_func_mulacc[1022] = mulacc_write_status_func;
    cx_func_mulacc[1023] = mulacc_read_status_func;
}