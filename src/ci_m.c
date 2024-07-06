// machine mode version of ci.c

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/ci.h"
#include "../include/utils.h"

#include "../zoo/mulacc/mulacc_common.h"
#include "../zoo/muldiv/muldiv_common.h"
#include "../zoo/addsub/addsub_common.h"
#include "../zoo/p-ext/p-ext_common.h"

#define CX_AVAIL_STATE 1
#define CX_UNAVAIL_STATE 0

typedef struct {
  // static
  int cx_guid;
  int num_states;

  // dynamic
  int *avail_state_ids;
} cx_entry_m_t;

typedef cx_entry_m_t cx_map_t;
cx_map_t cx_map[NUM_CX];


static inline cx_sel_t gen_cx_sel(cx_id_t cx_id, state_id_t state_id, 
                                  int32_t cx_version) 
{
    cx_selidx_t cx_sel = {.sel = {.cx_id = cx_id, 
                                  .state_id = state_id,
                                  .version = cx_version}};
    return cx_sel.idx;
}

static int get_free_state(cx_id_t cx_id) {
    for (int i = 0; i < cx_map[cx_id].num_states; i++) {
        if (cx_map[cx_id].avail_state_ids[i] == CX_AVAIL_STATE) {
            return i;
        }
    }
    return -1;
}

static int is_valid_state_id(cx_id_t cx_id, state_id_t state_id) 
{
    if (state_id < 0) {
        return false; // No available states for cx_guid 
    } else if (state_id > cx_map[cx_id].num_states - 1) {
        return false;
    }
    return true;
}

// populates the cx_map
void cx_init() {
    // zero the mcx_selector
    cx_csr_write(MCX_SELECTOR, 0);
    
    // 0 initialize the cx_status csr
    cx_csr_write(CX_STATUS, 0);

    cx_map[0].cx_guid = CX_GUID_MULDIV;
    cx_map[1].cx_guid = CX_GUID_ADDSUB;
    cx_map[2].cx_guid = CX_GUID_MULACC;
    cx_map[3].cx_guid = CX_GUID_PEXT;


    cx_map[0].num_states = CX_MULDIV_NUM_STATES;
    cx_map[1].num_states = CX_ADDSUB_NUM_STATES;
    cx_map[2].num_states = CX_MULACC_NUM_STATES;
    cx_map[3].num_states = CX_PEXT_NUM_STATES;

    for (int i = 0; i < NUM_CX; i++) {
        cx_map[i].avail_state_ids = malloc(cx_map[i].num_states * sizeof(int));
        for (int j = 0; j < cx_map[i].num_states; j++) {
            cx_map[i].avail_state_ids[j] = CX_AVAIL_STATE;
        }
    }
}

void cx_sel(int cx_sel) {
   cx_csr_write(MCX_SELECTOR, cx_sel);
}

int32_t cx_open(cx_guid_t cx_guid, cx_share_t cx_share) {
    cx_id_t cx_id = -1;
    for (int j = 0; j < NUM_CX; j++) {
        if (cx_map[j].cx_guid == cx_guid) {
                cx_id = j;
        }
    }
    
    if (cx_id == -1) {
       return -1; 
    }

    if (cx_map[cx_id].num_states == 0) {
        return gen_cx_sel(cx_id, 0, MCX_VERSION);
    } else {

        // Store the previous value in the cx_index csr
        cx_sel_t prev_sel = cx_csr_read(MCX_SELECTOR);

        state_id_t state_id = get_free_state(cx_id);
        
        if (!is_valid_state_id(cx_id, state_id)) {
            return -1;
        }

        cx_sel_t new_cx_sel = gen_cx_sel(cx_id, state_id, MCX_VERSION);

        cx_sel(new_cx_sel);

        uint status = CX_READ_STATUS();
        uint state_size = GET_CX_STATE_SIZE(status);

        if (state_size > 1023 || state_size < 0) {
            return -1;
        }

        cx_map[cx_id].avail_state_ids[state_id] = CX_UNAVAIL_STATE;

        uint sw_init = GET_CX_INITIALIZER(status);
        CX_WRITE_STATUS(INITIAL);

        // hw required to set to dirty after init, while sw does it explicitly
        if (sw_init) {
            for (int i = 0; i < state_size; i++) {
                CX_WRITE_STATE(i, 0);
            }
            CX_WRITE_STATUS(DIRTY);
        }

        cx_sel(prev_sel);
        return new_cx_sel;
    }
}


void cx_close(cx_sel_t cx_sel)
{
  cx_id_t cx_id = GET_CX_ID(cx_sel);
  // Stateless cx's
  if (cx_map[cx_id].num_states == 0) {
    return;
  // Stateful
  } else {
    state_id_t state_id = GET_CX_STATE(cx_sel);
    cx_map[cx_id].avail_state_ids[state_id] = CX_AVAIL_STATE;
  }
}

cx_error_t cx_error_read() {
  return cx_csr_read(CX_STATUS);
}

void cx_error_clear() {
  cx_csr_write(CX_STATUS, 0);
}