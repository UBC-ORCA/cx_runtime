#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mulacc_func.h"
#include "../../include/utils.h"

static int acc[CX_NUM_MULACC_INSTANCES][CX_MULACC_NUM_STATES];

static const cx_stctxs_t initial_status_word = {.sel = {.dc = CX_PRECLEAN,
                                                        .state_size = 1,
                                                        .reserved0 = 0,
                                                        .version = 1,
                                                        .R = 0}};

static const cx_stctxs_t off_status_word = {.sel = {.dc = CX_OFF,
                                                        .state_size = 1,
                                                        .reserved0 = 0,
                                                        .version = 1,
                                                        .R = 0}};

static cx_stctxs_t cxu_stctx_status[CX_NUM_MULACC_INSTANCES][CX_MULACC_NUM_STATES];

static int cxu_id_to_instance_num(int cx_id) {
    switch (cx_id)
    {
    case 2:
        return 0;
    default:
        printf("invalid cx_id in mulacc unit\n");
        exit(1);
    }
    return 0;
}

static inline int32_t do_nothing(__attribute__((unused)) int32_t a,
                                 __attribute__((unused)) int32_t b, 
                                 __attribute__((unused)) cx_selidx_t sys_sel) {
    return 0;
}

static inline int32_t mac_func(int32_t a, int32_t b, cx_selidx_t sys_sel)
{
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    cxu_stctx_status[inst_num][state_id].sel.dc = CX_DIRTY;
    acc[inst_num][state_id] += a * b;
    return acc[inst_num][state_id];
}

static inline int32_t reset_func(__attribute__((unused)) int32_t a,
                                 __attribute__((unused)) int32_t b, 
                                 cx_selidx_t sys_sel)
{
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    acc[inst_num][state_id] = 0;
    return 0;
}

static inline int32_t zero_mac_func(int32_t a, int32_t b, cx_selidx_t sys_sel)
{
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    cxu_stctx_status[inst_num][state_id].sel.dc = CX_CLEAN;
    int res = acc[inst_num][state_id] + a * b;
    acc[inst_num][state_id] = 0;
    return res;
}

static inline int32_t read_acc_func( __attribute__((unused)) int32_t unused0, 
                                     __attribute__((unused)) int32_t unused1, 
                                              cx_selidx_t sys_sel ) {
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    cxu_stctx_status[inst_num][state_id].sel.dc = CX_CLEAN;
    int res = acc[inst_num][state_id];
    acc[inst_num][state_id] = 0;
    return res;
}

static inline int32_t mulacc_read_status_func( __attribute__((unused)) int32_t unused0, 
                                               __attribute__((unused)) int32_t unused1, 
                                               cx_selidx_t sys_sel ) {
    
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    return cxu_stctx_status[inst_num][state_id].idx;
};

static inline int32_t mulacc_write_status_func( int32_t value, 
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
        reset_func(0, 0, sys_sel);
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

static inline int32_t mulacc_read_state_func( __attribute__((unused)) int32_t unused0, 
                                              int32_t index, 
                                              cx_selidx_t sys_sel ) {
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    switch (index) {
        case 0:
            return acc[inst_num][state_id];
            break;
        default: 
            return 0xFFFFFFFF;
    };
}

static inline int32_t mulacc_write_state_func( int32_t index, 
                                               int32_t value, 
                                               cx_selidx_t sys_sel ) {
    int inst_num = cxu_id_to_instance_num(sys_sel.sel.cx_id);
    int state_id = sys_sel.sel.state_id;
    switch (index) {
        case 0:
            acc[inst_num][state_id] = value;
            break;
        default: 
            return 0xFFFFFFFF;
    };
};

int32_t (*cx_func_mulacc[MAX_CF_IDS]) (int32_t, int32_t, cx_selidx_t) = {
    mac_func,
    reset_func,
    do_nothing,
    zero_mac_func,
    read_acc_func
};

// TODO: This should be moved to another file
static int32_t cx_func_undefined (int32_t, int32_t, cx_selidx_t) { return -1; }

void init_cx_func_mulacc() {
    for (int i = CX_MULACC_NUM_FUNCS; i < MAX_CF_IDS - 4; i++) {
        cx_func_mulacc[i] = cx_func_undefined;
    }

    for (int j = 0; j < CX_NUM_MULACC_INSTANCES; j++) {
        for (int i = 0; i < CX_MULACC_NUM_STATES; i++) {
            cxu_stctx_status[j][i] = off_status_word;
        }
    }

    cx_func_mulacc[1020] = mulacc_write_state_func;
    cx_func_mulacc[1021] = mulacc_read_state_func;
    cx_func_mulacc[1022] = mulacc_write_status_func;
    cx_func_mulacc[1023] = mulacc_read_status_func;
}