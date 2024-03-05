#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/ci.h"
#include "../include/queue.h"
#include "../include/parser.h"

#define CXU_ID_BITS 8
#define STATE_ID_BITS 8
#define MAX_CXU_ID 1 << CXU_ID_BITS
#define MAX_STATE_ID 1 << STATE_ID_BITS
#define CX_SEL_TABLE_NUM_ENTRIES 1024

#define NUM_CX_IDS  2
#define NUM_CXUS    2
#define MAX_NUM_STATES 10

#define MCX_SELECTOR 0xBC0
#define CX_STATUS    0x801
#define MCX_TABLE    0xBC1
#define CX_INDEX     0x800

#define MCX_VERSION 0

typedef enum {
  OFF,
  INITIAL,
  CLEAN,
  DIRTY
} CS_STATUS_t;

static inline cx_sel_t write_state_id(cx_sel_t cx_sel, state_id_t state_id) 
{
    cx_sel &= ~0x00ff0000;
    cx_sel |= (state_id << 16) & 0x00ff0000;
    return cx_sel;
}

static inline cx_sel_t write_version(cx_sel_t cx_sel, int32_t cx_version) 
{
    cx_sel &= ~0xf0000000;
    cx_sel |= (cx_version << 28) & 0xf0000000;
    return cx_sel;
}

static inline cx_sel_t gen_cx_sel(cx_id_t cx_id, state_id_t state_id, 
                                  int32_t cx_version) 
{
    cx_sel_t cx_sel = cx_id;
    cx_sel = write_state_id(cx_sel, state_id);
    cx_sel = write_version(cx_sel, cx_version);
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
    /* TODO: Check for some kind of error */
    write_mcfx_selector(cx_sel_table[cx_sel_index]);
    /* TODO: Check for an error here too? */
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
    cx_guid_t  cx_guid;
    queue_t    *avail_state_ids;
    int32_t    counter; // open guid = increment, close guid = decrement
    int32_t    num_state_ids;
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
    }

    return;
}

void init_cfu_runtime() {
    avail_table_indices = make_queue(CX_SEL_TABLE_NUM_ENTRIES);

    // the first slot in the table is never available - reserved
    // for legacy cx_indicies
    dequeue(avail_table_indices);

    init_cx_map();
    return;
}

// int32_t cfu_reg(int32_t a, int32_t b, int32_t cf_id)
// {
//     int32_t result;
//     asm volatile (
//         "        cfu_reg 1,%0,%1,%2;\n"
//         : "=r" (result)
//         : "r" (a), "r" (b)
//         : 
//         );
//     return result;
// }
 
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
    cx_sel_t cx_sel = 0x0;
    // Should check the CPU to see if the cx_selector_table
    // is available
    #ifdef M_MODE

    return cx_sel;

    #else

    for (int32_t i = 0; i < NUM_CX_IDS; i++) {
        if (cx_map[i].cx_guid == cx_guid) {
            if (cx_map[i].num_state_ids == 0) {
                // TODO: make sure this is right
                // Checking if cx_sel_table already contains stateless cx_id
                for (int32_t j = 0; j < CX_SEL_TABLE_NUM_ENTRIES; j++) {
                    cx_id_t cx_id = cx_sel_table[j] & ~(~0 << 8);
                    if (cx_id == i) {
                        return j;
                    }
                }
                cx_sel = gen_cx_sel(i, 0, MCX_VERSION);
            } else {
                state_id_t state_id = dequeue(cx_map[i].avail_state_ids);
                if (state_id < 0) {
                    printf("No available states for cx_guid %d\n", cx_map[i].cx_guid);
                    exit(1);
                }
                cx_sel = gen_cx_sel(i, state_id, MCX_VERSION);
            }
            cx_map[i].counter++;
            break;
        }
    }

    cx_sel_t cx_index = dequeue(avail_table_indices);

    if (cx_index < 0) {
        printf("Error: No available cx_index_table slots (1024 in use)\n");
        exit(1);
    }

    cx_sel_table[cx_index] = cx_sel;

    return cx_index;

    #endif
}

void cx_close(cx_sel_t cx_sel)
{
    // TODO: should test to see if table is available (m mode vs u/s mode)
    // then, disable the context
    #ifdef M_MODE

    #else
    cx_sel_t cx_sel_entry = cx_sel_table[cx_sel];
    cx_id_t cx_id = cx_sel_entry & 0xFF;

    // TODO: make sure this is right
    state_id_t state_id = (cx_sel >> 16) & ~(~0 << 8);

    // let the state be used again
    enqueue(cx_map[cx_id].avail_state_ids, state_id);

    // keep track of number of open contexts for a given cx_guid
    cx_map[cx_id].counter--;

    // clear the table
    cx_sel_table[cx_sel] = 0;

    #endif
    enqueue(avail_table_indices, cx_sel);
    return;
}

void cx_deselect_and_close(cx_sel_t cx_sel)
{
    return;
}
