#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "../include/ci.h"

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

// Can also check the cx_sel_indicies
// int32_t verify_counters()
// {
//     int32_t *counters = calloc(NUM_CX_IDS, sizeof(int32_t));

//     for (int32_t i = 0; i < CX_SEL_TABLE_NUM_ENTRIES - 1; i++) {
//         cx_sel_t cx_sel = cx_sel_table[i];
//         if (cx_sel == 0) {
//             continue;
//         }
//         cx_id_t cx_id = ((cx_selidx_t) cx_sel).sel.cx_id;
//         assert(cx_id < NUM_CX_IDS);
//         counters[cx_id]++;
//     }

//     for (int32_t cx_id = 0; cx_id < NUM_CX_IDS; cx_id++) {
//         // stateless
//         if (cx_map[cx_id].num_state_ids == 0) {
//             // Only 1 stateless cx_index per cx_table allowed
//             if (counters[cx_id] > 1) {
//                 free(counters);
//                 return false;
//             }
//             // hanging value in cx_table
//             if (cx_map[cx_id].counter == 0 && counters[cx_id] == 1) {
//                 free(counters);
//                 return false;
//             }
//             // counter value was not properly decremented on close
//             if (cx_map[cx_id].counter > 0 && counters[cx_id] == 0) {
//                 free(counters);
//                 return false;
//             }
//         }

//         // stateful
//         else if (cx_map[cx_id].counter != counters[cx_id]) {
//             free(counters);
//             return false;
//         }
//     }
//     free(counters);
//     return true;
// }

int cx_sel(int cx_index) {
   asm volatile (
    "csrw 0x011, %0        \n\t" // TODO: Should be 800
    :
    : "r" (cx_index)
    :
   );
   return 0;
}

int32_t cx_open(cx_guid_t cx_guid, cx_share_t cx_share) {
  int cx_index = -1;
  asm volatile (
    "li a7, 457;        \n\t"  // syscall 51, cx_open
    "mv a0, %0;         \n\t"  // a0-a5 are ecall args 
    "ecall;             \n\t"
    "mv %1, a0;         \n\t" 
    :  "=r" (cx_index)
    :  "r"  (cx_guid)
    :
  );
  if (cx_index == -1) {
    printf("error: cx_sel = -1\n");
    exit(0);
  }
  return cx_index;
}

void cx_close(cx_sel_t cx_sel)
{
  int cx_close_error = -1;
  asm volatile (
    "li a7, 459;        \n\t"  // syscall 51, cx_open
    "mv a0, %0;         \n\t"  // a0-a5 are ecall args 
    "ecall;             \n\t"
    "mv %1, a0;         \n\t" 
    :  "=r" (cx_close_error)
    :  "r"  (cx_sel)
    : 
  );

  if (cx_close_error == -1) {
    printf("error: with cx_close\n");
    exit(0);
  }
}

void cx_deselect_and_close(cx_sel_t cx_sel)
{
    return;
}
