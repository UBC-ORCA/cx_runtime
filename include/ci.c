#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ci.h"
#include "queue.h"

#define CXU_ID_BITS 8
#define STATE_ID_BITS 8
#define MAX_CXU_ID 1 << CXU_ID_BITS
#define MAX_STATE_ID 1 << STATE_ID_BITS

#define NUM_CX_IDS 2
#define MAX_NUM_STATES 10

#define MCX_SELECTOR 0xBC0
#define CX_STATUS    0x801
#define MCX_TABLE    0xBC1
#define CX_INDEX     0x800

// cx_sel &= ~0x00ff0000;
// cx_sel |= (1 << 16) & 0x00ff0000;


// maps the cx_guid to its cx_id
typedef struct {
    cx_guid_t cx_guid;
    cx_id_t cx_id;
    state_id_t state_id;
} cx_map_t;

static cx_sel_t cx_sel_table[4096];
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

static cx_sel_t cx_select(cx_sel_t cx_sel) {
    return 0;
}

static cx_sel_t cx_open(cx_guid_t cx_guid, cx_share_t cx_share) 
{
    cx_sel_t prev_cx_sel = 0x0;
    cx_sel_t cx_sel = 0x0;
    for (int32_t i = 0; i < NUM_CX_IDS; i++) {
        if (cx_map[i].cx_guid == cx_guid) {
            break;
        }
    }

    /* TODO: Check to see if in M mode or U mode to decide on
             writing to mcfx_selector or cx_selector */
    asm volatile (
        "        csrrw %0, 0xBC0, %1;\n"     
        : 
        :  "r" (prev_cx_sel), "r" (cx_sel)
        : 
        );

    return prev_cx_sel;
}

static void cx_close(cx_sel_t cx_sel) 
{
    return;
}

static void cx_deselect_and_close(cx_sel_t cx_sel)
{
    return;
}