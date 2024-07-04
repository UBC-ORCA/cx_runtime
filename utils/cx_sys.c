// linux/cx_sys/

#include <linux/queue.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/module.h>

#include <linux/list.h>

#include "../../../../research/riscv-tools/cx_runtime/include/utils.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/mulacc/mulacc_common.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/muldiv/muldiv_common.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/addsub/addsub_common.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/p-ext/p-ext_common.h"

#define CX_SEL_TABLE_NUM_ENTRIES 1024

#define CX_VERSION 1
#define CX_STATE_AVAIL 1
#define CX_STATE_UNAVAIL 0

typedef int cx_id_t;
typedef int cx_sel_t;
typedef int state_id_t;

extern cx_entry_t cx_map[NUM_CX];

static inline cx_sel_t gen_cx_sel(cx_id_t cx_id, state_id_t state_id, uint cxe,
                                  int32_t cx_version) 
{
    cx_selidx_t cx_sel = {.sel = {.cx_id = cx_id, 
                                  .state_id = state_id,
                                  .cxe = cxe,
                                  .version = cx_version}};
    return cx_sel.idx;
}

static int get_free_state(int cx_id) {
        int state_id = front(cx_map[cx_id].avail_state_ids);
        if (state_id >= 0) {
                return state_id;
        }
        return -1;
}

static int is_valid_counter(cx_id_t cx_id) 
{
    int32_t counter = cx_map[cx_id].counter[0];
    // stateless
    if (/* counter == INT32_MAX || */ counter < 0) {
        return false;
    
    // stateful + counter out of range
    } else if (cx_map[cx_id].num_states > 0 && counter >= cx_map[cx_id].num_states) {
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
    } else if (state_id > cx_map[cx_id].num_states - 1) {
        return false;
    }
    return true;
}

static int initialize_state(uint status) 
{
        // 4. Read the state to get the state_size
        uint state_size = GET_CX_STATE_SIZE(status);

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

static void copy_state_to_os( uint state_size, uint index ) 
{ 
        for (int i = 0; i < state_size; i++) {
                current->cx_os_state_table[index].data[i] = CX_READ_STATE(i);
        }
}

static void copy_state_from_os( uint index ) 
{
        cx_os_state_t src = current->cx_os_state_table[index];
        for (int i = 0; i < src.size; i++) {
                CX_WRITE_STATE(i, src.data[i]);
        }
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
                if (cx_map[j].cx_guid == cx_guid) {
                        cx_id = j;
                }
        }

        if (cx_id == -1) {
                return -1;
        }

        if (cx_share < 0 || cx_share > GLOBAL_SHARED) {
                return -1;
        }

        // if (!is_valid_counter(cx_id)) {
        //         return -1;
        // }

        int cx_sel = -1;

        // stateless cx - checking if the value is in the cx sel table already
        if (cx_map[cx_id].num_states == 0) {
                cx_map[cx_id].counter[0]++;
                if (cx_map[cx_id].index[0] > 0) {
                        return cx_map[cx_id].index[0];
                }
                cx_map[cx_id].index[0] = cx_index;

                dequeue(current->cx_table_avail_indices);
                cx_sel = gen_cx_sel(cx_id, 0, 0, CX_VERSION);
                current->mcx_table[cx_index] = cx_sel;
        
        // stateful cx
        } else {
                int state_id = -1;
                if (cx_share == PROCESS_SHARED) {
                        // When we get PCIe device struct as a cx_map, this will be a lot more efficent
                        for (int i = 0; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
                                if (current->mcx_table[i] == CX_INVALID_SELECTOR) {
                                        continue;
                                }

                                // If there is already a state id for this cx_id that enables
                                // sharing
                                cx_selidx_t curr_selidx = { .idx = current->mcx_table[i] };
                                if ( curr_selidx.sel.cx_id == cx_id && 
                                     current->cx_os_state_table[i].share == PROCESS_SHARED ) {
                                        state_id = curr_selidx.sel.state_id;
                                }
                        }
                }
                if (state_id == -1) {

                        state_id = get_free_state(cx_id);
                        
                        if (!is_valid_state_id(cx_id, state_id)) {
                                // TODO: errno
                                // errno = 139;
                                return -1; // No available states for cx_guid 
                        }

                        dequeue(cx_map[cx_id].avail_state_ids);
                }
                BUG_ON ( state_id < 0 );
                dequeue(current->cx_table_avail_indices);
                cx_map[cx_id].counter[state_id]++;

                cx_sel = gen_cx_sel(cx_id, state_id, 1, CX_VERSION);
                current->mcx_table[cx_index] = cx_sel;

                // 2. Store the previous value in the cx_index csr
                cx_sel_t prev_sel_index = cx_csr_read(CX_INDEX);

                // check if previous selector value is valid
                if (prev_sel_index > 1023 || prev_sel_index < 0) {
                        return -1;
                }
                
                // 3. Update cx_index to the new value
                cx_csr_write( CX_INDEX, cx_index );

                // 4 + 5
                uint status = CX_READ_STATUS();
                
                int failure = initialize_state(status);
                if (failure) {
                        pr_info("there was a failure all along!\n");
                        return -1;
                }

                // 6. Update os information
                current->cx_os_state_table[cx_index].data = kzalloc(MAX_STATE_SIZE, GFP_KERNEL);
                current->cx_os_state_table[cx_index].share = GET_SHARE_TYPE(cx_share);
                current->cx_os_state_table[cx_index].size = GET_CX_STATE_SIZE(status);

                // 7. write the previous selector
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
        cx_id_t cx_id = ((cx_selidx_t) ((uint)cx_sel_entry)).sel.cx_id;

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
        if (cx_map[cx_id].num_states > 0) {
                state_id_t state_id = ((cx_selidx_t) ((uint)cx_sel_entry)).sel.state_id;
                if (!is_valid_state_id(cx_id, state_id)) {
                        // errno = 139;
                        return -1;
                }

                // keep track of number of open contexts for a given cx_guid
                cx_map[cx_id].counter[state_id]--;

                // clear the table
                current->mcx_table[cx_sel] = CX_INVALID_SELECTOR;

                // delete cx_sel_map_index from map
                // cx_map[cx_id].cx_sel_map_index[state_id] = -1;

                // Free from the OS
                kfree(current->cx_os_state_table[cx_sel].data);
                current->cx_os_state_table[cx_sel].size = 0;
                current->cx_os_state_table[cx_sel].ctx_status = 0;
                current->cx_os_state_table[cx_sel].ctx_status = 0;
                BUG_ON( cx_map[cx_id].counter[state_id] < 0 );

                // let the state be used again
                if (cx_map[cx_id].counter[state_id] == 0) {
                        enqueue(cx_map[cx_id].avail_state_ids, state_id);
                }
                enqueue(current->cx_table_avail_indices, cx_sel);

        // Stateless cx's
        } else if (cx_map[cx_id].num_states == 0) {

                cx_map[cx_id].counter[0] -= 1;
                
                // Don't clear the cx_selector_table entry unless the counter is at 0
                if (cx_map[cx_id].counter[0] == 0) {
                        current->mcx_table[cx_sel] = CX_INVALID_SELECTOR;
                        
                        enqueue(current->cx_table_avail_indices, cx_map[cx_id].index[0]);
                        cx_map[cx_id].index[0] = -1;
                }
        } else {
                pr_info("made it to case that shouldn't happen\n");
                return -1; // Shouldn't make it to this case
        }

        return 0;
}


SYSCALL_DEFINE0(context_save)
{
        // Save the cx_index
        cx_sel_t cx_sel_index = cx_csr_read(CX_INDEX);
        current->cx_index = cx_sel_index;

        for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
        
                // save the state data
                if (current->mcx_table[i] != CX_INVALID_SELECTOR) {

                        // write the index to be saved
                        cx_csr_write(CX_INDEX, i);
                        
                        uint cx_status = CX_READ_STATUS();

                        // Don't need to save data if it's not dirty
                        if (GET_CX_STATUS(cx_status) != DIRTY) {
                                continue;
                        }
                        uint state_size = GET_CX_STATE_SIZE(cx_status);
                        
                        // copy state data to OS
                        copy_state_to_os(state_size, i);

                        // set the state context back to its initial state
                        int failure = initialize_state(cx_status);

                        if (failure) {
                                return -1;
                        }

                        // save the state context status
                        current->cx_os_state_table[i].ctx_status = cx_status;

                        cx_stctxs_t cx_stctx = {.idx = cx_status};
                        // save the new size
                        current->cx_os_state_table[i].size = cx_stctx.sel.state_size;
                }
        }

        uint cx_error = cx_csr_read(CX_STATUS);

        current->cx_status = cx_error;

        cx_csr_write(CX_INDEX, current->cx_index);

        return 0;
}

SYSCALL_DEFINE0(context_restore)
{
        // 1. Restore error
        csr_write(CX_STATUS, current->cx_status);
        
        // 2. Restore mcx_table
        csr_write(MCX_TABLE, &current->mcx_table[0]);

        // 3. Restore state information
        for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
                
                cx_stctxs_t cx_stctxs = {.idx = current->cx_os_state_table[i].ctx_status};

                // ignore table indicies that aren't occupied, and only save dirty states
                if (current->mcx_table[i] != CX_INVALID_SELECTOR) {

                        // Write the index to be restored
                        cx_csr_write( CX_INDEX,  i );
                        
                        // Restore state
                        copy_state_from_os( i );

                        // Restoring status word + setting to clean
                        cx_stctxs.sel.cs = CLEAN;
                        current->cx_os_state_table[i].ctx_status = cx_stctxs.idx;
                        CX_WRITE_STATUS(cx_stctxs.idx);
                }
        }

        // 4. Restore index
        cx_csr_write( CX_INDEX, current->cx_index );

        return 0;
}

SYSCALL_DEFINE0(do_nothing)
{       

        // Save the current state index
        uint cx_index_A = cx_csr_read(CX_INDEX);

        // Update the mcx_table to clear cxe bit for current selector
        // This needs to be early in the case that we double trap, which we will when we do as soon
        // as we execute the following CX_READ_STATUS.
        uint cx_sel_A = current->mcx_table[cx_index_A];
        cx_sel_A &= ~(1 << (CX_CXE_START_INDEX));
        current->mcx_table[cx_index_A] = cx_sel_A;

        csr_write(MCX_SELECTOR, cx_sel_A);

        // Because we're trapping on first use, the status we read does not belong to the
        // cx_index, but rather the index with the cxe == 0 bit in the mcx_table.
        cx_stctxs_t cx_stctxs_B = {.idx = CX_READ_STATUS()};

        cx_selidx_t cx_selidx_A = {.idx = current->mcx_table[cx_index_A]};

        for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
                
                if (current->mcx_table[i] == CX_INVALID_SELECTOR) {
                        continue;
                }
                // Because we've already updated the mcx_table, don't restore
                // the state info of the current state.
                if (i == cx_index_A) {
                        continue;
                }
                cx_selidx_t cx_selidx_B = {.idx = current->mcx_table[i]};

                // Virtualize same state / cx_id only
                if (!(cx_selidx_B.sel.cx_id == cx_selidx_A.sel.cx_id && 
                    cx_selidx_B.sel.state_id == cx_selidx_A.sel.state_id)) {
                        continue;
                }

                // Don't need to save and restore stateless cxs
                if (cx_map[GET_CX_ID(current->mcx_table[i])].num_states == 0) {
                        continue;
                }

                uint cxe = GET_CX_CXE(current->mcx_table[i]);
                if (!cxe) {
                           
                        // Write the index to be saved
                        cx_csr_write(CX_INDEX, i);

                        // Storing status word + setting to clean
                        cx_stctxs_B.sel.cs = CLEAN;
                        current->cx_os_state_table[i].ctx_status = cx_stctxs_B.idx;

                        // Update the mcx_table to set cxe bit from prev selector
                        uint cx_sel_B = current->mcx_table[i];
                        cx_sel_B |= (1 << (CX_CXE_START_INDEX));
                        current->mcx_table[i] = cx_sel_B;

                        // Storing state
                        copy_state_to_os( cx_stctxs_B.sel.state_size, i );

                        // Restore current correct state index
                        cx_csr_write( CX_INDEX, cx_index_A );

                        // Restore state information + Update state context status information
                        // Only if this data has been saved before e.g., if we aren't coming from a cx_open
                        if (GET_CX_STATUS(current->cx_os_state_table[cx_index_A].ctx_status) > OFF) {
                                copy_state_from_os( cx_index_A );
                                CX_WRITE_STATUS( current->cx_os_state_table[cx_index_A].ctx_status );
                        }
                        break;
                }
        }

        return 0;
}