#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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

#define FALSE 0
#define TRUE 1

#define MCX_SELECTOR 0xBC0
#define CX_STATUS    0x801
#define MCX_TABLE    0xBC1
#define CX_INDEX     0x800

#define MCX_VERSION 0

typedef union {
     struct {
         uint32_t version   : 4;
         uint32_t reserved0 : 4;
         uint32_t state_id  : 8;
         uint32_t reserved1 : 8;
         uint32_t cx_id     : 8;
     } sel;
     int32_t idx;
 } cx_selidx_t;


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

static int32_t NUM_CX_IDS = 0;
static int32_t NUM_CXUS   = 0;

static cx_sel_t cx_sel_table[CX_SEL_TABLE_NUM_ENTRIES];
static queue_t *avail_table_indices;

static inline cx_sel_t gen_cx_sel(cx_id_t cx_id, state_id_t state_id, 
                                  int32_t cx_version) 
{
    cx_sel_t cx_sel = cx_id;
    cx_sel = SET_STATE_ID(cx_sel, state_id);
    cx_sel = SET_VERSION(cx_sel, cx_version);
    return cx_sel;
}

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

// index # is the cx_id
typedef struct {
    // static members
    cx_guid_t  cx_guid;
    int32_t    num_state_ids;

    // dynamic members
    queue_t    *avail_state_ids;
    int32_t    *cx_sel_index; // keeps track of cx_table_indicies
    int32_t    counter; // open guid = increment, close guid = decrement
} cx_map_t;

static cx_map_t *cx_map;

void init_cx_map() 
{
    cx_config_info_t cx_config_info = read_files("");

    cx_map = (cx_map_t *) malloc(sizeof(cx_map_t) * cx_config_info.num_cxs);
    NUM_CX_IDS = cx_config_info.num_cxs;

    for (int32_t i = 0; i < cx_config_info.num_cxs; i++) {
        cx_map[i].cx_guid = cx_config_info.cx_config[i].cx_guid;
        cx_map[i].avail_state_ids = make_queue(cx_config_info.cx_config[i].num_states);
        cx_map[i].counter = 0;
        cx_map[i].num_state_ids = cx_config_info.cx_config[i].num_states;

        // in the case of a stateless cx, still need to allocate 1 slot
        int32_t num_cx_sel_indicies = (cx_config_info.cx_config[i].num_states == 0) ? 
            1 : cx_config_info.cx_config[i].num_states; 

        cx_map[i].cx_sel_index = (int32_t *) malloc(sizeof(int32_t) * num_cx_sel_indicies);

        for (int32_t j = 0; j < num_cx_sel_indicies; j++) {
            cx_map[i].cx_sel_index[j] = -1;
        }
    }

    return;
}

int32_t is_valid_cx_sel_index(cx_sel_t cx_sel) 
{
    if (cx_sel > CX_SEL_TABLE_NUM_ENTRIES - 1) {
        return FALSE;
    } else if (cx_sel == 0) {
        return FALSE;
    }
    return TRUE;
}

int32_t is_valid_counter(cx_id_t cx_id) 
{
    int32_t counter = cx_map[cx_id].counter;
    if (counter == INT32_MAX || counter < 0) {
        return FALSE;
    
    // stateful + counter out of range
    } else if (cx_map[cx_id].num_state_ids > 0 && counter >= cx_map[cx_id].num_state_ids) {
        return FALSE;
    }
    return TRUE;
}

int32_t is_valid_state_id(cx_id_t cx_id, state_id_t state_id) 
{
    if (state_id < 0) {
        return FALSE; // No available states for cx_guid 
    } else if (state_id > cx_map[cx_id].num_state_ids - 1) {
        return FALSE;
    }
    return TRUE;
}

int32_t is_valid_cx_id(cx_id_t cx_id) 
{
    if (cx_id < 0) {
        return FALSE; // cx_id not found
    }

    if (cx_id > NUM_CX_IDS) {
        return FALSE; // cx_id not in valid range
    }

    return TRUE;
}

int32_t is_valid_cx_table_sel(cx_sel_t cx_sel)
{
    if (cx_sel < 1 || cx_sel > CX_SEL_TABLE_NUM_ENTRIES - 1) {
        return FALSE;
    }
    return TRUE;
}

int32_t verify_counters()
{
    int32_t *counters = malloc(sizeof(int32_t) * NUM_CX_IDS);

    for (int32_t i = 0; i < CX_SEL_TABLE_NUM_ENTRIES - 1; i++) {
        cx_sel_t cx_sel = cx_sel_table[i];
        if (cx_sel == 0) {
            continue;
        }
        cx_id_t cx_id = GET_CX_ID(cx_sel);
        assert(cx_id < NUM_CX_IDS);
        counters[cx_id]++;
    }
    for (int32_t cx_id = 0; cx_id < NUM_CX_IDS; cx_id++) {
        if (cx_map[cx_id].counter != counters[cx_id]) {
            free(counters);
            return FALSE;
        }
    }
    free(counters);
    return TRUE;
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

    #if M_MODE

    prev_cx_sel = write_mcfx_selector(cx_sel);

    #else

    prev_cx_sel = write_cx_selector_index(cx_sel);

    #endif

    return prev_cx_sel;
}


/*
* Allocates an index on the cx_selector_table.
* Stateless cxs: in the case there is already an index allocated, increment the counter and 
*                return the cx_table_index. Otherwise, allocate an index and increment the counter.
* Stateful cxs:  allocate an unused state to a free cx_selector_table index.
*/
cx_sel_t cx_open(cx_guid_t cx_guid, cx_share_t cx_share) 
{
    cx_sel_t cx_sel = 0;
    
    // Should check the CPU to see if the cx_selector_table
    // is available
    #if M_MODE

    return cx_sel;

    #else

    // Check to see if there is a free cx_sel_table index
    cx_sel_t cx_index = front(avail_table_indices);

    if (cx_index < 0) {
        return -1; // No available cx_index_table slots (1024 in use)
    }

    int32_t cx_id = -1;
    for (int32_t i = 0; i < NUM_CX_IDS; i++) {
        if (cx_map[i].cx_guid == cx_guid) {
            cx_id = i;
            break;
        }
    }

    if (!is_valid_cx_id(cx_id)) {
        return -1;
    }

    if (!is_valid_counter(cx_id)) {
        return -1;
    }

    state_id_t state_id = front(cx_map[cx_id].avail_state_ids);

    // stateless function - checking if the value is in the cx sel table already
    if (cx_map[cx_id].num_state_ids == 0) {
        
        if (cx_map[cx_id].cx_sel_index[0] > 0) {
            
            if (!is_valid_cx_sel_index(cx_map[cx_id].cx_sel_index[0])) {
                return -1;
            }

            cx_map[cx_id].counter++;
            return cx_map[cx_id].cx_sel_index[0];
        }
        cx_sel = gen_cx_sel(cx_id, 0, MCX_VERSION);
    // stateful function
    } else {
                
        if (!is_valid_state_id(cx_id, state_id)) {
            return -1; // No available states for cx_guid 
        }

        dequeue(cx_map[cx_id].avail_state_ids);
        cx_sel = gen_cx_sel(cx_id, state_id, MCX_VERSION);
    }
    
    #if DEBUG

    if (!verify_counters()) {
        return -1;
    }

    #endif

    cx_map[cx_id].counter++;
    
    dequeue(avail_table_indices);
    cx_sel_table[cx_index] = cx_sel;

    if (cx_map[cx_id].num_state_ids == 0) {
        cx_map[cx_id].cx_sel_index[0] = cx_index;
    } else {
        cx_map[cx_id].cx_sel_index[state_id] = cx_index;
    }

    return cx_index;

    #endif
}

/*
* Stateless cxs: Decrements counter. In the case that the last instance of 
                 a cx has been closed, the cx is removed from the cx_sel_table.
* Stateful cxs:  Removes entry from cx_sel_table and decrements the counter. 
                 Marks state as available.
*/
void cx_close(cx_sel_t cx_sel)
{

    // TODO: should test to see if table is available (m mode vs u/s mode)
    // then, disable the context
    #if M_MODE

    #else
    
    if (!is_valid_cx_table_sel(cx_sel)) {
        return;
    }

    cx_sel_t cx_sel_entry = cx_sel_table[cx_sel];

    cx_id_t cx_id = GET_CX_ID(cx_sel_entry);

    if (!is_valid_cx_id(cx_id)) {
        return;
    };

    #if DEBUG

    if (!verify_counters()) {
        return -1;
    }

    #endif

    // Stateful cx's
    if (cx_map[cx_id].num_state_ids > 0) {
        state_id_t state_id = GET_STATE_ID(cx_sel);
        if (!is_valid_state_id(cx_id, state_id)) {
            return;
        }

        // keep track of number of open contexts for a given cx_guid
        cx_map[cx_id].counter -= 1;

        // clear the table
        cx_sel_table[cx_sel] = 0;

        // delete cx_sel_index from map
        cx_map[cx_id].cx_sel_index[state_id] = -1;

        // let the state be used again
        // TODO: There will need to be mutual exculsion in the case with multiple threads
        enqueue(cx_map[cx_id].avail_state_ids, state_id);
        enqueue(avail_table_indices, cx_sel);

    // Stateless cx's
    } else if (cx_map[cx_id].num_state_ids == 0) {
        cx_map[cx_id].counter -= 1;
        
        // Don't clear the cx_selector_table entry unless the counter is at 0
        if (cx_map[cx_id].counter == 0) {
            cx_sel_table[cx_sel] = 0;
            cx_map[cx_id].cx_sel_index[0] = -1;
        }
    } else {
        return; // Shouldn't make it to this case
    }

    #endif
    return;
}

void cx_deselect_and_close(cx_sel_t cx_sel)
{
    return;
}
