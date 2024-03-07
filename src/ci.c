#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/ci.h"
#include "../include/queue.h"
#include "../include/parser.h"

#define CX_ID_BITS 8
#define CX_ID_START_INDEX 0
#define STATE_ID_BITS 8
#define STATE_ID_START_INDEX 16
#define MAX_CXU_ID 1 << CX_ID_BITS
#define MAX_STATE_ID 1 << STATE_ID_BITS
#define CX_SEL_TABLE_NUM_ENTRIES 1024
#define VERSION_START_INDEX 28

#define NUM_CX_IDS  2
#define NUM_CXUS    2

#define MCX_SELECTOR 0xBC0
#define CX_STATUS    0x801
#define MCX_TABLE    0xBC1
#define CX_INDEX     0x800

#define MCX_VERSION 0

#define GET_BITS(cx_sel, start_bit, n) \
    (cx_sel >> start_bit) & (((1 << n) - 1) )

#define GET_CX_ID(cx_sel) \
    GET_BITS(cx_sel, CX_ID_START_INDEX, CX_ID_BITS)

#define GET_STATE_ID(cx_sel) \
    GET_BITS(cx_sel, STATE_ID_START_INDEX, STATE_ID_BITS)

#define SET_STATE_ID(cx_sel, state_id) \
    (cx_sel & 0xff00ffff) | ((state_id << STATE_ID_START_INDEX) & 0x00ff0000)

#define SET_VERSION(cx_sel, version) \
    (cx_sel & 0x0fffffff) | ((version << VERSION_START_INDEX) & 0xf0000000)

typedef enum {
  OFF,
  INITIAL,
  CLEAN,
  DIRTY
} CS_STATUS_t;

static inline cx_sel_t gen_cx_sel(cx_id_t cx_id, state_id_t state_id, 
                                  int32_t cx_version) 
{
    cx_sel_t cx_sel = cx_id;
    cx_sel = SET_STATE_ID(cx_sel, state_id);
    cx_sel = SET_VERSION(cx_sel, cx_version);
    return cx_sel;
}

static cx_sel_t cx_sel_table[CX_SEL_TABLE_NUM_ENTRIES];
static cxu_state_context_status_t cxu_state_context_status[NUM_CXUS];
static queue_t *avail_table_indices;

static inline cx_sel_t write_mcfx_selector(cx_sel_t cx_sel)
{
    cx_sel_t prev_cx_sel = 0x0;
    asm volatile (
        "        csrrw %0, 0xBC0, %1;\n"
        :  "=r" (prev_cx_sel)
        :  "r"  (cx_sel)
        : 
        );
    return prev_cx_sel;
}

static inline cx_sel_t write_cx_selector_index(cx_sel_t cx_sel_index) 
{
    cx_sel_t prev_cx_sel_index = 0x0;
    asm volatile (
        "        csrrw %0, 0x800, %1;\n"     
        :  "=r" (prev_cx_sel_index)
        :  "r"  (cx_sel_index)
        : 
        );
    write_mcfx_selector(cx_sel_table[cx_sel_index]);
    return prev_cx_sel_index;
}

/* Context specific functions + structs */

// TODO: These context r/w insts should be a CSRs, but for now we're just going
//       to write to an array of cxu status registers
static inline cxu_state_context_status_t read_context_status(
    cxu_state_context_status_t cxu_state_context_status)
{
    return cxu_state_context_status & 0b11;
}

static inline void write_context_status(
    cxu_state_context_status_t cxu_state_context_status,
    CS_STATUS_t cs_status)
{
    cxu_state_context_status &= ~0b11;
    cxu_state_context_status |= (cs_status << 2) & 0b11;
    return;
}

static inline cxu_state_context_status_t read_context_size (
    cxu_state_context_status_t cxu_state_context_status)
{
    return (cxu_state_context_status >> 2) & 0x1111111111;
}

static inline void write_context_size(
    cxu_state_context_status_t cxu_state_context_status,
    int32_t state_size)
{
    cxu_state_context_status &= ~0b111111111100;
    cxu_state_context_status |= (state_size << 2) & 0b111111111100;

    return;
}

/* End of context specific functions + structs */

// index # is the cx_id
typedef struct {
    // static members
    cx_guid_t  cx_guid;
    int32_t    num_state_ids;

    // dynamic members
    queue_t    *avail_state_ids;
    int32_t    counter; // open guid = increment, close guid = decrement
    int32_t    cx_sel_index; // keeps track of cx_table_index for stateless cx's
} cx_map_t;

static cx_map_t *cx_map;

void init_cx_map() 
{
    cx_config_info_t cx_config_info = read_files("");

    cx_map = (cx_map_t *) malloc(sizeof(cx_map_t) * cx_config_info.num_cxs);

    for (int32_t i = 0; i < cx_config_info.num_cxs; i++) {
        cx_map[i].cx_guid = cx_config_info.cx_config[i].cx_guid;
        cx_map[i].avail_state_ids = make_queue(cx_config_info.cx_config[i].num_states);
        cx_map[i].counter = 0;
        cx_map[i].num_state_ids = cx_config_info.cx_config[i].num_states;
        cx_map[i].cx_sel_index = -1;
    }

    return;
}

void cx_init() {
    avail_table_indices = make_queue(CX_SEL_TABLE_NUM_ENTRIES);

    // the first slot in the table is never available - reserved
    // for legacy cx_indicies
    dequeue(avail_table_indices);

    init_cx_map();
    return;
}
 
cx_sel_t cx_select(cx_sel_t cx_sel) {

    cx_sel_t prev_cx_sel = 0;

    #ifdef M_MODE

    prev_cx_sel = write_mcfx_selector(cx_sel);

    #else

    prev_cx_sel = write_cx_selector_index(cx_sel);

    #endif

    return prev_cx_sel;
}

cx_sel_t cx_open(cx_guid_t cx_guid, cx_share_t cx_share) 
{
    cx_sel_t cx_sel = 0;
    
    // Should check the CPU to see if the cx_selector_table
    // is available
    #ifdef M_MODE

    return cx_sel;

    #else

    // Check to see if there is a free cx_sel_table index
    cx_sel_t cx_index = front(avail_table_indices);

    if (cx_index < 0) {
        printf("Error: No available cx_index_table slots (1024 in use)\n");
        return cx_index; // number returned will relate to error
    }

    int32_t cx_id = -1;
    for (int32_t i = 0; i < NUM_CX_IDS; i++) {
        if (cx_map[i].cx_guid == cx_guid) {
            cx_id = i;
            // stateless function - checking if the value is in the cx sel table already
            if (cx_map[i].num_state_ids == 0) {
                if (cx_map[i].cx_sel_index > 0) {
                    return cx_map[i].cx_sel_index;
                }
                cx_sel = gen_cx_sel(i, 0, MCX_VERSION);
            } else {
                // stateful function
                state_id_t state_id = front(cx_map[i].avail_state_ids);
                if (state_id < 0) {
                    return cx_index; // No available states for cx_guid 
                }
                cx_sel = gen_cx_sel(i, state_id, MCX_VERSION);
            }
            break;
        }
    }
    
    // TODO: write a debug function that counts the entries in the cx_sel_table
    //       to make sure it aligns with the counter.
    cx_map[cx_id].counter++; // this could error out if overflowed

    dequeue(avail_table_indices);
    dequeue(cx_map[cx_id].avail_state_ids);

    cx_sel_table[cx_index] = cx_sel;

    return cx_index;

    #endif
}

// TODO: what exactly am I doing in this function?!
void cx_close(cx_sel_t cx_sel)
{

    // TODO: should test to see if table is available (m mode vs u/s mode)
    // then, disable the context
    #ifdef M_MODE

    #else
    cx_sel_t cx_sel_entry = cx_sel_table[cx_sel];

    // TODO: make sure that this is a valid state_id (e.g., within the num_states,
    // as a sanity check) before enqeueing

    // TODO: We should have another sanity check so that the counter matches the 
    // number of open states for each cx library. These should be true before (precondition)
    // and after (postcondition) e.g., before and after a loop. If this condiditon is inside 
    // of the loop, it is called a loop invariant. Can assume this is true every iteration of
    // the loop.

    cx_id_t cx_id = GET_CX_ID(cx_sel_entry);

    // TODO: make sure this is right
    state_id_t state_id = GET_STATE_ID(cx_sel);

    // keep track of number of open contexts for a given cx_guid
    // -- or ++ may lead to race conditions.
    cx_map[cx_id].counter--;

    // clear the table
    cx_sel_table[cx_sel] = 0;

    // let the state be used again
    // TODO: There will need to be mutual exculsion in the case with multiple threads
    enqueue(cx_map[cx_id].avail_state_ids, state_id);

    #endif
    enqueue(avail_table_indices, cx_sel);
    return;
}

void cx_deselect_and_close(cx_sel_t cx_sel)
{
    return;
}
