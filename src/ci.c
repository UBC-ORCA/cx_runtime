#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "../include/ci.h"
#include "../include/queue.h"

#define CXU_ID_BITS 8
#define STATE_ID_BITS 8
#define MAX_CXU_ID 1 << CXU_ID_BITS
#define MAX_STATE_ID 1 << STATE_ID_BITS
#define CX_SEL_TABLE_SIZE 4096

#define NUM_CX_IDS  2
#define NUM_CXUS    2
#define MAX_NUM_STATES 10

#define MCX_SELECTOR 0xBC0
#define CX_STATUS    0x801
#define MCX_TABLE    0xBC1
#define CX_INDEX     0x800

#define STATE_CONTEXT_0 0x7C0
#define STATE_CONTEXT_1 0x7C1


#define MCX_VERSION 0

const cx_guid_t CX_GUID_A = 15;
const cx_guid_t CX_GUID_B = 20;

static enum CS_STATUS {
  OFF,
  INITIAL,
  CLEAN,
  DIRTY
};

/* Should take in a pointer 
*  Should be a shift - the cx_id will be an index of an entry in the map
*/
static inline cx_sel_t write_cx_id(cx_sel_t cx_sel, cx_id_t cx_id) 
{
    return cx_id;
}

/* Should take in a pointer
*  Should fail if there are no available states
*/
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

// index # is the cx_id
typedef struct {
    cx_guid_t cx_guid;
    state_id_t state_id; // will at some point be # of state_ids, or a 
                         // list of available state_ids
} cx_map_t;

static cx_sel_t cx_sel_table[CX_SEL_TABLE_SIZE];
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
static inline cxu_state_context_status_t read_context_status(
    cxu_state_context_status_t cxu_state_context_status)
{
    return 0;
}

static inline cxu_state_context_status_t write_context_status()
{
    return 0;
}

static inline cxu_state_context_status_t read_context_size()
{
    return 0;
}

static inline cxu_state_context_status_t write_context_size()
{
    return 0;
}

/* End of context specific functions + structs */

void init_cfu_runtime() {
    avail_table_indices = make_queue();
    return;
}

cx_map_t static cx_map[NUM_CX_IDS] = {
    {CX_GUID_A, 0},
    {CX_GUID_B, 1}
};

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
    for (int32_t i = 0; i < NUM_CX_IDS; i++) {
        if (cx_map[i].cx_guid == cx_guid) {
            cx_sel = gen_cx_sel(i, cx_map[i].state_id, MCX_VERSION);
            break;
        }
    }

    // Should check the CPU to see if the cx_selector_table
    // is available
    #ifdef M_MODE

    return cx_sel;

    #else

    cx_sel_t cx_index = dequeue(avail_table_indices);
    cx_sel_table[cx_index] = cx_sel;

    return cx_index;

    #endif
}

void cx_close(cx_sel_t cx_sel)
{
    // TODO: should test to see if table is available (m mode vs u/s mode)
    // then, disable the context
    enqueue(avail_table_indices, cx_sel);
    return;
}

void cx_deselect_and_close(cx_sel_t cx_sel)
{
    return;
}
