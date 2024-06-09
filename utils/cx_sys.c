// linux/cx_sys/

#include <linux/queue.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

#include "../../../../research/riscv-tools/cx_runtime/include/utils.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/mulacc/mulacc_common.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/muldiv/muldiv_common.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/addsub/addsub_common.h"

#define CX_SEL_TABLE_NUM_ENTRIES 1024

#define CX_VERSION 1
#define CX_STATE_AVAIL 1
#define CX_STATE_UNAVAIL 0

typedef int cx_id_t;
typedef int cx_sel_t;
typedef int state_id_t;

static inline cx_sel_t gen_cx_sel(cx_id_t cx_id, state_id_t state_id, 
                                  int32_t cx_version) 
{
    cx_selidx_t cx_sel = {.sel = {.cx_id = cx_id, 
                                  .state_id = state_id,
                                  .version = cx_version}};
    return cx_sel.idx;
}

static int get_free_state(int cx_id) {
        int state_id = front(current->cx_map[cx_id].avail_state_ids);
        if (state_id >= 0) {
                return state_id;
        }
        return -1;
}

static int is_valid_counter(cx_id_t cx_id) 
{
    int32_t counter = current->cx_map[cx_id].counter;
    // stateless
    if (/* counter == INT32_MAX || */ counter < 0) {
        return false;
    
    // stateful + counter out of range
    } else if (current->cx_map[cx_id].num_states > 0 && counter >= current->cx_map[cx_id].num_states) {
        return false;
    }
    return true;
}

static int is_valid_cx_id(cx_id_t cx_id) 
{
    if (cx_id < 0) {
        return false; // cx_id not found
    }

    if (cx_id > NUM_CX) {
        return false; // cx_id not in valid range
    }
    return true;
}

static int is_valid_cx_table_sel(cx_sel_t cx_sel)
{
    if (cx_sel < 1 || cx_sel > CX_SEL_TABLE_NUM_ENTRIES - 1) {
        return false;
    }
    return true;
}

static int is_valid_state_id(cx_id_t cx_id, state_id_t state_id) 
{
    if (state_id < 0) {
        return false; // No available states for cx_guid 
    } else if (state_id > current->cx_map[cx_id].num_states - 1) {
        return false;
    }
    return true;
}

static int initialize_state(uint status) 
{
        // 4. Read the state to get the state_size
        uint state_size = 1025;
        state_size = GET_CX_STATE_SIZE(status);

        if (state_size > 1023 || state_size < 0) {
                return 1;
        }

        // 5. Set the CXU to initial state
        uint sw_init = GET_CX_INITIALIZER(status);

        CX_WRITE_STATUS(INITIAL);

        // hw required to set to dirty after init, while sw does it explicitly
        if (sw_init) {
                for (int i = 0; i < state_size; i++) {
                        CX_WRITE_STATE(i, 0);
                }
                CX_WRITE_STATUS(DIRTY);
        }
        return 0;
}

// Can also check the cx_sel_indicies
// static int verify_counters()
// {
//     int *counters = calloc(NUM_CX_IDS, sizeof(int32_t));

//     for (int i = 0; i < CX_SEL_TABLE_NUM_ENTRIES - 1; i++) {
//         cx_sel_t cx_sel = cx_sel_table[i];
//         if (cx_sel == 0) {
//             continue;
//         }
//         cx_id_t cx_id = ((cx_selidx_t) cx_sel).sel.cx_id;
//         assert(cx_id < NUM_CX_IDS);
//         counters[cx_id]++;
//     }

//     for (int cx_id = 0; cx_id < NUM_CX_IDS; cx_id++) {
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

// static void reset_state( int32_t state_id, int32_t state_size ) {
//         for (int i = 0; i < state_size; i++) {
//                 int a = 0;
//                 CX_WRITE_STATE(i, a);
//         }
// }

static void copy_state_to_os( int cx_index, cx_os_state_t dest ) 
{
        // Save the previous index
        cx_sel_t prev_sel_index = cx_csr_read(CX_INDEX);

        uint cx_sel = current->mcx_table[prev_sel_index];

        // write the new index
        cx_csr_write(MCX_SELECTOR, cx_index);
        
        for (int i = 0; i < dest.size; i++) {
                CX_READ_STATE(dest.data[i], i);
        }

        uint status = 0xFFFFFFFF;
        CX_READ_STATUS(status);
        dest.ctx_status = status;

        int failure = initialize_state(status);
        if (failure) {
                return;
        }
        // restore previous index
        cx_csr_write(CX_INDEX, prev_sel_index);
}

static void copy_state_from_os( cx_os_state_t src, int cx_index ) {
        
        // write the new index
        cx_csr_write(CX_INDEX, cx_index);

        for (int i = 0; i < src.size; i++) {
                CX_WRITE_STATE(i, src.data[i]);
        }
}

/*
* Initializes the cx_map, cx_table, and sets the CX_INDEX, MCX_SELECTOR, and CX_STATUS CSRs 
* to their initial values (0).
*/
SYSCALL_DEFINE0(cx_init)
{
        current->mcx_table = kzalloc(sizeof(int) * CX_SEL_TABLE_NUM_ENTRIES, GFP_KERNEL);
        if (!current->mcx_table) {
                // some sort of allocation error here
                return -1;
        }

        current->cx_os_state_table = kzalloc(sizeof(cx_os_state_t) * CX_SEL_TABLE_NUM_ENTRIES, GFP_KERNEL);
        if (!current->cx_os_state_table) {
                // some sort of allocation error here
                return -1;
        }

        // TODO: Check for successful allocation
        current->cx_table_avail_indices = make_queue(CX_SEL_TABLE_NUM_ENTRIES);

        // 1st slot in table is canonical legacy value
        current->mcx_table[0] = CX_LEGACY;
        dequeue(current->cx_table_avail_indices);

        for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
                current->mcx_table[i] = CX_INVALID_SELECTOR;
        }

        // Update the mcx_table csr with the mcx_table address
        cx_csr_write(MCX_TABLE, &current->mcx_table[0]);

        // 0 initialize the cx_index table csr
        cx_csr_write(CX_INDEX, 0);

        // 0 initialize the mcx_selector csr
        cx_csr_write(MCX_SELECTOR, 0);

        // 0 initialize the cx_status csr
        cx_csr_write(CX_STATUS, 0);
        
        current->cx_map[0].cx_guid = CX_GUID_MULDIV;
        current->cx_map[1].cx_guid = CX_GUID_ADDSUB;
        current->cx_map[2].cx_guid = CX_GUID_MULACC;

        current->cx_map[0].num_states = CX_MULDIV_NUM_STATES;
        current->cx_map[1].num_states = CX_ADDSUB_NUM_STATES;
        current->cx_map[2].num_states = CX_MULACC_NUM_STATES;
        
        int32_t num_states = -1;
        for (int i = 0; i < NUM_CX; i++) {

                num_states = current->cx_map[i].num_states;

                // stateless cxu
                if (num_states == 0) {
                        current->cx_map[i].index = kzalloc(sizeof(int) * 1, GFP_KERNEL);
                        current->cx_map[i].index[0] = -1;
                }
                
                // stateful cxu
                else {
                        current->cx_map[i].avail_state_ids = make_queue(num_states);
                }

                current->cx_map[i].counter = 0;
        }
        return 0;
}

/*
* Allocates an index on the mcx_table.
* Stateless cxs: in the case there is already an index allocated, increment the counter and 
*                return the cx_index. Otherwise, allocate an index and increment the counter.
* Stateful cxs:  allocate an unused state to a free cx_table index.
*/
SYSCALL_DEFINE2(cx_open, int, cx_guid, int, cx_share)
{
        // 1. Check if we have the resources necessary to open a new entry on the scx_table
        // Check to see if there is a free cx_sel_table index
        int cx_index = front(current->cx_table_avail_indices);

        if (cx_index < 1) {
                return -1;
        }

        int cx_id = -1;

        for (int j = 0; j < NUM_CX; j++) {
                if (current->cx_map[j].cx_guid == cx_guid) {
                        cx_id = j;
                }
        }

        if (cx_id == -1) {
                return -1;
        }

        // if (!is_valid_counter(cx_id)) {
        //         return -1;
        // }

        int cx_sel = -1;

        // stateless cx - checking if the value is in the cx sel table already
        if (current->cx_map[cx_id].num_states == 0) {
                current->cx_map[cx_id].counter++;
                if (current->cx_map[cx_id].index[0] > 0) {
                        return current->cx_map[cx_id].index[0];
                }
                current->cx_map[cx_id].index[0] = cx_index;

                dequeue(current->cx_table_avail_indices);
                cx_sel = gen_cx_sel(cx_id, 0, CX_VERSION);
                current->mcx_table[cx_index] = cx_sel;
        
        // stateful cx
        } else {

                int state_id = get_free_state(cx_id);
                // pr_info("cx_id: %d, state_id: %d\n", cx_id, state_id);
                if (state_id < 0) {
                        return -1;
                }
                
                if (!is_valid_state_id(cx_id, state_id)) {
                        // TODO: errno
                        // errno = 139;
                        return -1; // No available states for cx_guid 
                }

                dequeue(current->cx_map[cx_id].avail_state_ids);
                dequeue(current->cx_table_avail_indices);
                current->cx_map[cx_id].counter++;

                cx_sel = gen_cx_sel(cx_id, state_id, CX_VERSION);
                current->mcx_table[cx_index] = cx_sel;

                // 2. Store the previous value in the cx_index csr
                cx_sel_t prev_sel_index = cx_csr_read(CX_INDEX);

                // check if previous selector value is valid
                if (prev_sel_index > 1023 || prev_sel_index < 0) {
                        return -1;
                }

                // 3. Update cx_index to the new value
                cx_csr_write(CX_INDEX, cx_index);

                // 4 + 5
                uint status = 0xFFFFFFFF;
                CX_READ_STATUS(status);
                int failure = initialize_state(status);
                if (failure) {
                        return -1;
                }

                // 6. Allocate memory in the OS for the state
                current->cx_os_state_table[cx_index].data = kzalloc(MAX_STATE_SIZE, GFP_KERNEL);
                current->cx_os_state_table[cx_index].share = GET_SHARE_TYPE(cx_share);
                current->cx_os_state_table[cx_index].size = GET_CX_STATE_SIZE(status);

                // 7. Write the previous selector value back to cx_index
                cx_csr_write(CX_INDEX, prev_sel_index);
        }
        return cx_index;
}

/*
* Stateless cxs: Decrements counter. In the case that the last instance of 
                 a cx has been closed, the cx is removed from the cx_table.
* Stateful cxs:  Removes entry from cx_table and decrements the counter. 
                 Marks state as available.
*/
SYSCALL_DEFINE1(cx_close, int, cx_sel)
{
        if (!is_valid_cx_table_sel(cx_sel)) {
                // TODO: should be the same error as man 2 close
                return -1;
        }

        cx_sel_t cx_sel_entry = current->mcx_table[cx_sel];

        if (cx_sel_entry == CX_INVALID_SELECTOR) {
                return -1;
        }

        // TODO: see if we can remove this cast
        cx_id_t cx_id = ((cx_selidx_t) cx_sel_entry).sel.cx_id;

        if (!is_valid_cx_id(cx_id)) {
                return -1;
        };

        #ifdef DEBUG

        if (!verify_counters()) {
                errno = 137;
                return -1;
        }
        #endif
        // Stateful cx's
        if (current->cx_map[cx_id].num_states > 0) {
                state_id_t state_id = ((cx_selidx_t) cx_sel).sel.state_id;
                if (!is_valid_state_id(cx_id, state_id)) {
                        // errno = 139;
                        return -1;
                }

                // keep track of number of open contexts for a given cx_guid
                current->cx_map[cx_id].counter -= 1;

                // clear the table
                current->mcx_table[cx_sel] = CX_INVALID_SELECTOR;

                // delete cx_sel_map_index from map
                // current->cx_map[cx_id].cx_sel_map_index[state_id] = -1;

                // Free from the OS
                kfree(current->cx_os_state_table[cx_sel].data);

                // let the state be used again
                enqueue(current->cx_map[cx_id].avail_state_ids, state_id);
                enqueue(current->cx_table_avail_indices, cx_sel);
                current->cx_map[cx_id].counter--;


        // Stateless cx's
        } else if (current->cx_map[cx_id].num_states == 0) {

                // State must be 0 in the table
                // if (((cx_selidx_t) cx_sel_entry).sel.state_id != 0) {
                //         // errno = 139;
                //         return -1;
                // }

                current->cx_map[cx_id].counter -= 1;
                
                // // Don't clear the cx_selector_table entry unless the counter is at 0
                if (current->cx_map[cx_id].counter == 0) {
                        current->mcx_table[cx_sel] = CX_INVALID_SELECTOR;
                        
                        enqueue(current->cx_table_avail_indices, current->cx_map[cx_id].index[0]);
                        current->cx_map[cx_id].index[0] = -1;
                }
        } else {
                return -1; // Shouldn't make it to this case
        }
        return 0;
}


SYSCALL_DEFINE0(context_save)
{
        cx_sel_t cx_sel_index = cx_csr_read(CX_INDEX);
        current->cx_index = cx_sel_index;

        for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
                // TODO: Will need to check to see if we have to save each state
                if (current->mcx_table[i] != CX_INVALID_SELECTOR) {
                        copy_state_to_os( current->mcx_table[i], current->cx_os_state_table[i]);
                }
        }

        uint cx_error = cx_csr_read(CX_STATUS);

        current->cx_status = cx_error;

        return 0;
}

SYSCALL_DEFINE0(context_restore)
{
        // 1. Restore index
        cx_csr_write(CX_INDEX, current->cx_index);

        // 2. Restore error
        cx_csr_write(CX_STATUS, current->cx_status);
        
        // 3. Restore mcx_table
        cx_csr_write(MCX_TABLE, &current->mcx_table[0]);

        // 4. Restore state information
        for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
                // TODO: Will need to check to see if we have to save each state
                if (current->mcx_table[i] != CX_INVALID_SELECTOR) {
                        copy_state_from_os( current->cx_os_state_table[i], i );
                }

                // Restoring status word + setting to clean
                cx_stctxs_t cx_stctxs = {{current->cx_os_state_table[i].ctx_status}};
                cx_stctxs.sel.cs = CLEAN;
                current->cx_os_state_table[i].ctx_status = cx_stctxs.idx;
                CX_WRITE_STATUS(cx_stctxs.idx);
        }

        return 0;
}