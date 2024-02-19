#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "ci.h"
#include "queue.h"

#define CXU_ID_BITS 8
#define STATE_ID_BITS 8
#define MAX_CXU_ID 1 << CXU_ID_BITS
#define MAX_STATE_ID 1 << STATE_ID_BITS
#define CX_SEL_TABLE_SIZE 4096

#define NUM_CX_IDS 2
#define MAX_NUM_STATES 10

#define MCX_SELECTOR 0xBC0
#define CX_STATUS    0x801
#define MCX_TABLE    0xBC1
#define CX_INDEX     0x800

#define MCX_VERSION 0

const cx_guid_t CX_GUID_A = 15;
const cx_guid_t CX_GUID_B = 20;

static inline state_id_t read_state_id(cx_sel_t cx_sel) 
{
    // read from bits 23 -> 16
    return (cx_sel >> 16) & ~(~0 << 8);
}

static inline cx_id_t read_cx_id(cx_sel_t cx_sel) 
{
    // read from bits 7 -> 0
    return cx_sel & ~(~0 << 8);
}

/* Should take in a pointer */
static inline cx_sel_t write_cx_id(cx_sel_t cx_sel, cx_id_t cx_id) 
{
    cx_sel &= ~0xff;
    cx_sel |= cx_id & 0xff;
    return cx_sel;
}

/* Should take in a pointer */
static inline cx_sel_t write_state_id(cx_sel_t cx_sel, state_id_t state_id) 
{
    cx_sel &= ~0x00ff0000;
    cx_sel |= (state_id << 16) & 0x00ff0000;
    return cx_sel;
}

static inline cx_sel_t write_version(cx_sel_t cx_sel, state_id_t state_id) 
{
    cx_sel &= ~0xf0000000;
    cx_sel |= (state_id << 28) & 0xf0000000;
    return cx_sel;
}

// maps the cx_guid to its cx_id
typedef struct {
    cx_guid_t cx_guid;
    cx_id_t cx_id;
    state_id_t state_id;
} cx_map_t;

static cx_sel_t cx_sel_table[CX_SEL_TABLE_SIZE];
static queue_t *avail_table_indices;

void init_cfu_runtime() {
    avail_table_indices = make_queue();

}

cx_map_t static cx_map[NUM_CX_IDS] = {
    {15, 5, 0},
    {20, 10, 1}
};

int32_t cfu_reg(int32_t a, int32_t b, int32_t cf_id)
{
    int32_t result;
    asm volatile (
        "        cfu_reg 1,%0,%1,%2;\n"
        : "=r" (result)
        : "r" (a), "r" (b)
        : 
        );
    return result;
}

cx_sel_t cx_select(cx_sel_t cx_index) {

    cx_sel_t cx_sel = cx_sel_table[cx_index];
    cx_sel_t prev_cx_sel = 0x0;

    asm volatile (
        "        csrrw %0, 0xBC0, %1;\n"     
        :  "=r" (prev_cx_sel)
        :  "r"  (cx_sel)
        : 
        );

    int32_t prev_cx_sel_index = -1;
    for (int32_t i = 0; i < NUM_CX_IDS; i++) {
        if (cx_sel_table[i] == prev_cx_sel) {
            prev_cx_sel_index = i;
            break;
        }
    }

    if (prev_cx_sel_index == -1) {
        printf("Error: couldn't find index for prev cx_sel in cx_sel_table\n");
    }

    return prev_cx_sel_index;
}

cx_sel_t cx_open(cx_guid_t cx_guid, cx_share_t cx_share) 
{
    cx_sel_t cx_sel = 0x0;
    for (int32_t i = 0; i < NUM_CX_IDS; i++) {
        if (cx_map[i].cx_guid == cx_guid) {
           cx_sel = write_cx_id(cx_sel, cx_map[i].cx_id);
           cx_sel = write_state_id(cx_sel, cx_map[i].state_id);
           cx_sel = write_version(cx_sel, MCX_VERSION);
           break;
        }
    }
    
    cx_sel_t cx_index = dequeue(avail_table_indices);

    cx_sel_table[cx_index] = cx_sel;

    /* TODO: Check to see if in M mode or U mode to decide on
             writing to mcfx_selector or cx_selector */
    cx_sel_t prev_cx_sel = 0x0;

    /* TODO: This should probably be a write to cx_selector, 
             which will always store the index into the table */
    asm volatile (
        "        csrrw %0, 0xBC0, %1;\n"     
        :  "=r" (prev_cx_sel) 
        :  "r" (cx_sel)
        : 
        );

    return cx_index;
}

void cx_close(cx_sel_t cx_sel)
{
    // cx_sel_table[cx_sel] = 0x0;
    enqueue(avail_table_indices, cx_sel);
    return;
}

void cx_deselect_and_close(cx_sel_t cx_sel)
{
    return;
}