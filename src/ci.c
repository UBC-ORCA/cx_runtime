#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>

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
#define DEBUG 1
#define UNASSIGNED_STATE -1

#define MCX_SELECTOR 0xBC0
#define CX_STATUS    0x801
#define MCX_TABLE    0x802 // should be 0xBC1
#define CX_INDEX     0x800

#define MCX_VERSION 1

typedef union {
     struct {
        uint32_t cx_id     : 8;
        uint32_t reserved1 : 8;
        uint32_t state_id  : 8;
        uint32_t reserved0 : 4;
        uint32_t version   : 4;
     } sel;
     cx_sel_t idx;
 } cx_selidx_t;

static int32_t NUM_CX_IDS = 0;
static int32_t NUM_CXUS   = 0;

static cx_sel_t cx_sel_table[CX_SEL_TABLE_NUM_ENTRIES];
static queue_t *avail_table_indices;

static inline cx_sel_t gen_cx_sel(cx_id_t cx_id, state_id_t state_id, 
                                  int32_t cx_version) 
{
    cx_selidx_t cx_sel = {.sel = {.cx_id = cx_id, 
                                  .state_id = state_id,
                                  .version = cx_version}};
    return cx_sel.idx;
}


static inline uint32_t* get_mcx_table_ptr()
{
    uint32_t mcx_table;
    asm volatile (
        "        csrr 0x802, %0;\n"
        : "=r"  (mcx_table)
        :
        );
    uint32_t *mcx_table_ptr = (uint32_t *)mcx_table;
    return mcx_table_ptr;
}

static inline cx_sel_t write_mcx_selector(cx_sel_t cx_sel)
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
    
    // should be done in hardware (spike)
    // when the cx_index is written to, we should read cx_sel_table[cx_sel_index],
    // and copy it into mcx_selector. 
    write_mcx_selector(cx_sel_table[cx_sel_index]);
    return prev_cx_sel_index;
}

static inline void read_mcx_table() 
{
    uint64_t val = 0x0;
    printf("val: %08x\n", &val);
    asm volatile (
            "        csrr %0, 0xBC1;\n"
            :  "=r"  (val)
            :
            );
    printf("mcx_table address: %d\n", val);
}

// index # is the cx_id
typedef struct {
    // static members
    cx_guid_t  cx_guid;
    int32_t    num_state_ids;

    // dynamic members
    queue_t    *avail_state_ids;

    // TODO: update this name
    int32_t    *cx_sel_map_index; // keeps track of cx_table_indicies
    // TODO: in the case of virtualized state_id's, this breaks. 
    // might be possible to move this to a kernel / OS structure that tells you 
    // per process who owns each state.  
    
    int32_t    counter; // open guid = increment, close guid = decrement
} cx_map_entry_t;

typedef cx_map_entry_t *cx_map_t;
static cx_map_t cx_map;

void init_cx_map(char **paths, int32_t num_cxs) 
{
    cx_map = (cx_map_t) malloc(sizeof(cx_map_entry_t) * num_cxs);
    /* Check and see if malloc fails, throw an error if it fails, write in spec
       what will happen if it fails. */
    
    cx_config_info_t cx_config_info = read_files(paths, num_cxs);

    NUM_CX_IDS = cx_config_info.num_cxs;

    for (int32_t i = 0; i < cx_config_info.num_cxs; i++) {
        cx_map[i].cx_guid = cx_config_info.cx_config[i].cx_guid;

        // Make sure that in the case of stateless cx's, this has a queue of size 1
        cx_map[i].avail_state_ids = make_queue(cx_config_info.cx_config[i].num_states);
        cx_map[i].counter = 0;
        cx_map[i].num_state_ids = cx_config_info.cx_config[i].num_states;

        // in the case of a stateless cx, still need to allocate 1 slot
        // TODO: indicies spelling
        int32_t num_cx_sel_indicies = (cx_config_info.cx_config[i].num_states == 0) ? 
            1 : cx_config_info.cx_config[i].num_states; 

        cx_map[i].cx_sel_map_index = (int32_t *) malloc(sizeof(int32_t) * num_cx_sel_indicies);

        // -1 means that it is an unassigned state_id
        // TODO: might be better to be a typed thing vs. enum
        for (int32_t j = 0; j < num_cx_sel_indicies; j++) {
            cx_map[i].cx_sel_map_index[j] = UNASSIGNED_STATE;
        }
    }
}

int32_t is_valid_cx_sel_index(cx_sel_t cx_sel) 
{
    if (cx_sel > CX_SEL_TABLE_NUM_ENTRIES - 1) {
        return false;
    } else if (cx_sel == 0) {
        return false;
    }
    return true;
}

// TODO; more descriptive name
int32_t is_valid_counter(cx_id_t cx_id) 
{
    int32_t counter = cx_map[cx_id].counter;
    // stateless
    if (counter == INT32_MAX || counter < 0) {
        return false;
    
    // stateful + counter out of range
    } else if (cx_map[cx_id].num_state_ids > 0 && counter >= cx_map[cx_id].num_state_ids) {
        return false;
    }
    return true;
}

int32_t is_valid_state_id(cx_id_t cx_id, state_id_t state_id) 
{
    if (state_id < 0) {
        return false; // No available states for cx_guid 
    } else if (state_id > cx_map[cx_id].num_state_ids - 1) {
        return false;
    }
    return true;
}

int32_t is_valid_cx_id(cx_id_t cx_id) 
{
    if (cx_id < 0) {
        return false; // cx_id not found
    }

    if (cx_id > NUM_CX_IDS) {
        return false; // cx_id not in valid range
    }
    return true;
}

int32_t is_valid_cx_table_sel(cx_sel_t cx_sel)
{
    if (cx_sel < 1 || cx_sel > CX_SEL_TABLE_NUM_ENTRIES - 1) {
        return false;
    }
    return true;
}

// Can also check the cx_sel_indicies
int32_t verify_counters()
{
    int32_t *counters = calloc(NUM_CX_IDS, sizeof(int32_t));

    for (int32_t i = 0; i < CX_SEL_TABLE_NUM_ENTRIES - 1; i++) {
        cx_sel_t cx_sel = cx_sel_table[i];
        if (cx_sel == 0) {
            continue;
        }
        cx_id_t cx_id = ((cx_selidx_t) cx_sel).sel.cx_id;
        assert(cx_id < NUM_CX_IDS);
        counters[cx_id]++;
    }

    for (int32_t cx_id = 0; cx_id < NUM_CX_IDS; cx_id++) {
        // stateless
        if (cx_map[cx_id].num_state_ids == 0) {
            // Only 1 stateless cx_index per cx_table allowed
            if (counters[cx_id] > 1) {
                free(counters);
                return false;
            }
            // hanging value in cx_table
            if (cx_map[cx_id].counter == 0 && counters[cx_id] == 1) {
                free(counters);
                return false;
            }
            // counter value was not properly decremented on close
            if (cx_map[cx_id].counter > 0 && counters[cx_id] == 0) {
                free(counters);
                return false;
            }
        }

        // stateful
        else if (cx_map[cx_id].counter != counters[cx_id]) {
            free(counters);
            return false;
        }
    }
    free(counters);
    return true;
}

void cx_init(char **paths, int32_t num_cxs) {
    avail_table_indices = make_queue(CX_SEL_TABLE_NUM_ENTRIES);

    // the first slot in the table is never available - reserved
    // for legacy cx_indicies
    dequeue(avail_table_indices);

    init_cx_map(paths, num_cxs);

    write_cx_selector_index(0);
    return;
}

cx_sel_t cx_select(cx_sel_t cx_sel) {

    cx_sel_t prev_cx_sel = 0;

    #if CX_SEL_TABLE

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
    #if CX_SEL_TABLE

    return cx_sel;

    #else

    // Check to see if there is a free cx_sel_table index
    cx_sel_t cx_index = front(avail_table_indices);

    if (cx_index < 0) {
        errno = 134;
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
        // This should be an errno, not a status register update.
        errno = 135;
        return -1;
    }

    if (!is_valid_counter(cx_id)) {
        errno = 136;
        return -1;
    }

    state_id_t state_id = front(cx_map[cx_id].avail_state_ids);

    // stateless cx - checking if the value is in the cx sel table already
    if (cx_map[cx_id].num_state_ids == 0) {
        
        if (cx_map[cx_id].cx_sel_map_index[0] > 0) {
            cx_map[cx_id].counter++;
            return cx_map[cx_id].cx_sel_map_index[0];
        }
        // TODO: consider moving the mcx_version to gen_cx_sel, and not pass it as a parameter
        cx_sel = gen_cx_sel(cx_id, 0, MCX_VERSION);
    
    // stateful cx
    } else {
                
        if (!is_valid_state_id(cx_id, state_id)) {
            // TODO: errno
            errno = 139;
            return -1; // No available states for cx_guid 
        }

        dequeue(cx_map[cx_id].avail_state_ids);
        cx_sel = gen_cx_sel(cx_id, state_id, MCX_VERSION);
    }
    
    #if DEBUG

    if (!verify_counters()) {
        errno = 137;
        return -1;
    }

    #endif

    cx_map[cx_id].counter++;
    
    dequeue(avail_table_indices);
    cx_sel_table[cx_index] = cx_sel;

    if (cx_map[cx_id].num_state_ids == 0) {
        cx_map[cx_id].cx_sel_map_index[0] = cx_index;
    } else {
        cx_map[cx_id].cx_sel_map_index[state_id] = cx_index;
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
    #if CX_SEL_TABLE

    return;

    #else
    
    if (!is_valid_cx_table_sel(cx_sel)) {
        // TODO: should be the same error as man 2 close
        return;
    }

    cx_sel_t cx_sel_entry = cx_sel_table[cx_sel];

    if (cx_sel_entry == 0) {
        errno = 140;
        return;
    }

    // TODO: see if we can remove this cast
    cx_id_t cx_id = ((cx_selidx_t) cx_sel_entry).sel.cx_id;

    if (!is_valid_cx_id(cx_id)) {
        errno = 135;
        return;
    };

    #if DEBUG

    if (!verify_counters()) {
        errno = 137;
        return;
    }

    #endif

    // Stateful cx's
    if (cx_map[cx_id].num_state_ids > 0) {
        state_id_t state_id = ((cx_selidx_t) cx_sel).sel.state_id;
        if (!is_valid_state_id(cx_id, state_id)) {
            errno = 139;
            return;
        }

        // keep track of number of open contexts for a given cx_guid
        cx_map[cx_id].counter -= 1;

        // clear the table
        cx_sel_table[cx_sel] = 0;

        // delete cx_sel_map_index from map
        cx_map[cx_id].cx_sel_map_index[state_id] = -1;

        // let the state be used again
        // TODO: There will need to be mutual exculsion in the case with multiple threads
        enqueue(cx_map[cx_id].avail_state_ids, state_id);
        enqueue(avail_table_indices, cx_sel);

    // Stateless cx's
    } else if (cx_map[cx_id].num_state_ids == 0) {

        // State must be 0 in the table
        if (((cx_selidx_t) cx_sel_entry).sel.state_id != 0) {
            errno = 139;
            return;
        }

        cx_map[cx_id].counter -= 1;
        
        // Don't clear the cx_selector_table entry unless the counter is at 0
        if (cx_map[cx_id].counter == 0) {
            cx_sel_table[cx_sel] = 0;
            cx_map[cx_id].cx_sel_map_index[0] = -1;
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
