#include <stdint.h>

#include "mulacc_func.h"
#include "../../include/utils.h"

static int acc[CX_MULACC_NUM_STATES];

static const cx_stctxs_t initial_status_word = {.sel = {.cs = INITIAL,
                                                        .state_size = 1,
                                                        .error = 0}};

static cx_stctxs_t cxu_stctx_status[CX_MULACC_NUM_STATES]; // set size to 1

static inline int32_t mac_func(int32_t a, int32_t b, int32_t state_id)
{
    cxu_stctx_status[state_id].sel.cs = DIRTY;
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
    // TODO: See if this is a good approach
    //       And see what to do for the other cases (off, dirty)
    int cx_status = GET_CX_STATUS(value);
    if (cx_status == INITIAL) {
        reset_func(0, 0, state_id);
        cxu_stctx_status[state_id] = initial_status_word;
    }
    else if (cx_status == CLEAN)
    {
        cxu_stctx_status[state_id].sel.cs = CLEAN;
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

    cx_stctxs_t off_status = {.sel = {.cs = OFF,
                                      .error = 0,
                                      .state_size = 1}};

    // TODO: This initialization should be done in cx_init... I think
    for (int i = 0; i < CX_MULACC_NUM_STATES; i++) {
        cxu_stctx_status[i] = off_status;
    }

    cx_func_mulacc[1020] = mulacc_write_state_func;
    cx_func_mulacc[1021] = mulacc_read_state_func;
    cx_func_mulacc[1022] = mulacc_write_status_func;
    cx_func_mulacc[1023] = mulacc_read_status_func;
}