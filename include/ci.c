#include <stdint.h>

#include "cx_runtime.h"

static cxu_id_t get_cxu(cx_id_t cx_id) 
{
    return cx_id;
}

// state context management
static state_id_t alloc_state(cxu_id_t cxu_id) 
{
    return cxu_id;
}
static void free_state(cxu_id_t cxu_id, state_id_t state_id)
{
    return;
}

// CX + state selectors
static cx_sel_t alloc_sel_cxu(cxu_id_t cxu_id) 
{
    return cx_id;
}
static cx_sel_t alloc_sel_cxu_state(cx_id_t) 
{
    return cx_id;
}
static void free_sel(cx_sel_t) 
{
    return;
}

// CX multiplexing
static cx_sel_t set_cur_sel(cx_sel_t)
{
    return cx_sel;
}
