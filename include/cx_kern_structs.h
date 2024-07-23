#ifndef KERN_STRUCTS_H
#define KERN_STRUCTS_H

// #include <stdint.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/queue.h>

#include "utils.h"

typedef s32 cx_guid_t;       // global: CX ID, a 128b GUID
typedef s32 cxu_guid_t;      // cxu package global name
typedef s32 cx_id_t;         // system: CXU index

typedef s32 state_id_t;      // system: state index
typedef s32 cx_share_t;      // context sharing permissions

typedef s32 cx_sel_t;        // hart: CX selector (value (No CX Table) or index
                             //       (when there is a CX Table))

typedef u32 cx_error_t;     // 

typedef s32 cxu_sctx_t;      // per state 

// typedef struct idx_t {
//     s32 idx;
//     struct list_head list;
// } idx_t;

// 1 state_data_t for each use of a given state context
typedef struct cx_state_data_t {
    s32 counter; // Needed for stateless, as can be more than 1 per thread
    u32 status;
    u32 *data;
} cx_state_data_t;

// 1 state_info_t struct per state context
typedef struct cx_state_info_t {
    CX_SHARE_T share;
    // when the counter is 0, we can set the CX_SHARE_T. Until it becomes
    // 0'ed again, we must resepect that all newly opened virtual
    // contexts are of the same share type, or else the cx_open will fail.
    s32 counter;
} cx_state_info_t;

// 1 cxu_info_t per cxu
typedef struct cxu_t {
    u32 num_cx;
    u32 *cx_guid; // each cxu can implement multiple cx's
    u32 num_states;
    queue_t *avail_state_ids;
    cx_state_info_t *state_info;
} cxu_t;

typedef struct {
  uint ctx_status;
  s32 counter; // only used for stateless cxs
  int *data;
} cx_os_state_t;

typedef struct {
  // static
  int cx_guid;
  int num_states;

  // dynamic
  queue_t *avail_state_ids;
  cx_state_info_t *state_info;
} cx_entry_t;

#endif // KERN_STRUCTS_H
