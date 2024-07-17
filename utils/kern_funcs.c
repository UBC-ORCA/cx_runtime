// linux/lib/
#include <linux/sched.h>
#include <linux/queue.h>
#include <linux/slab.h>
#include <linux/kern_funcs.h>

#include "../../../../research/riscv-tools/cx_runtime/include/cx_kern_structs.h"
#include "../../../../research/riscv-tools/cx_runtime/include/utils.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/mulacc/mulacc_common.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/muldiv/muldiv_common.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/addsub/addsub_common.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/p-ext/p-ext_common.h"

extern cx_entry_t cx_map[NUM_CX];

int cx_process_alloc(struct task_struct *tsk) {
	tsk->mcx_table = kzalloc(sizeof(int) * CX_SEL_TABLE_NUM_ENTRIES, GFP_KERNEL);
	if (tsk->mcx_table == NULL) {
			pr_info("failed mcx_table allocation\n");
			return -1;
	}
	// pr_info("table addr: %08x\n", &tsk->mcx_table);

	tsk->cx_os_state_table = (cx_os_state_t *)kzalloc(sizeof(cx_os_state_t) * CX_SEL_TABLE_NUM_ENTRIES, GFP_KERNEL);
	if (tsk->cx_os_state_table == NULL) {
		pr_info("failed cx_os_state_table allocation\n");
		return -1;
	}

	tsk->cx_table_avail_indices = alloc_queue(CX_SEL_TABLE_NUM_ENTRIES);
	if (tsk->cx_table_avail_indices == NULL) {
		pr_info("failed cx_table_avail_indices allocation\n");
		return -1;
	}

	return 0;
}

int cx_init_process(struct task_struct *tsk) {
	// 1st slot in table is canonical legacy value
	tsk->mcx_table[0] = CX_LEGACY;

	for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
		tsk->mcx_table[i] = CX_INVALID_SELECTOR;
		enqueue(tsk->cx_table_avail_indices, i);
	}

	return 0;
}

int cx_copy_table(struct task_struct *new) {
	// 1st slot in table is canonical legacy value
	if (!new->mm)
		return 0;
	pr_info("copying table\n");

	new->mcx_table[0] = CX_LEGACY;

	for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
		cx_sel_t prev_cx_sel = current->mcx_table[i];
		if (prev_cx_sel == CX_INVALID_SELECTOR) {
			enqueue(new->cx_table_avail_indices, i);
			new->mcx_table[i] = prev_cx_sel;
			continue;
		}
		cxu_guid_t cxu_id = GET_CX_ID(prev_cx_sel);
		state_id_t state_id = GET_CX_STATE(prev_cx_sel);

		cx_share_t cx_share = cx_map[cxu_id].state_info[state_id].share;
		
		if (cx_share == EXCLUDED) {
			pr_info("excluded cx; i: %d, sel: %08x\n", i, prev_cx_sel);
			return -1;
		} else {
			new->mcx_table[i] = prev_cx_sel;
		}
	}

	/* Should be the previous ones */
	new->cx_index = current->cx_index;
	new->cx_status = current->cx_status;
	return 0;
}

int cx_init(void) {
		
        pr_info("Ran in part of main\n");
		int retval = cx_process_alloc(current);
        if (retval < 0) {
			pr_info("error in cx_init\n");
			return -1;
		}

		cx_init_process(current);

		// Update the mcx_table csr with the mcx_table address
		csr_write(MCX_TABLE, &current->mcx_table[0]);

		// 0 initialize the cx_index table csr
		csr_write(CX_INDEX, 0);

		// 0 initialize the mcx_selector csr
		csr_write(MCX_SELECTOR, 0);

		// 0 initialize the cx_status csr
		csr_write(CX_STATUS, 0);

        cx_map[0].cx_guid = CX_GUID_MULDIV;
        cx_map[1].cx_guid = CX_GUID_ADDSUB;
        cx_map[2].cx_guid = CX_GUID_MULACC;
        cx_map[3].cx_guid = CX_GUID_PEXT;

        cx_map[0].num_states = CX_MULDIV_NUM_STATES;
        cx_map[1].num_states = CX_ADDSUB_NUM_STATES;
        cx_map[2].num_states = CX_MULACC_NUM_STATES;
        cx_map[3].num_states = CX_PEXT_NUM_STATES;

        int32_t num_states = -1;

        for (int i = 0; i < NUM_CX; i++) {

                num_states = cx_map[i].num_states;

                // stateless cxu
                if (num_states == 0) {
						cx_map[i].state_info = (cx_state_info_t *) kzalloc(sizeof(cx_state_info_t), GFP_KERNEL);
                        cx_map[i].state_info[0].index = kzalloc(sizeof(int) * 1, GFP_KERNEL);
						cx_map[i].state_info[0].index[0] = -1;
                }
                // stateful cxu
                else {
                        // cx_map[i].index = kzalloc(sizeof(int) * 1, GFP_KERNEL);
                        cx_map[i].state_info = (cx_state_info_t *) kzalloc(sizeof(cx_state_info_t) * num_states, GFP_KERNEL);
						for (int j = 0; j < num_states; j++) {
							cx_map[i].state_info[j].share = -1;
							cx_map[i].state_info[j].counter = 0;
						}
                        cx_map[i].avail_state_ids = make_queue(num_states);
                }
        }

        return 0;
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

int is_valid_state_id(cx_id_t cx_id, state_id_t state_id) 
{
    if (state_id < 0) {
        return false; // No available states for cx_guid 
    } else if (state_id > cx_map[cx_id].num_states - 1) {
        return false;
    }
    return true;
}

int cx_close(struct task_struct *tsk, int cx_sel) 
{
	if (!is_valid_cx_table_sel(cx_sel)) {
		// TODO: should be the same error as man 2 close
		return -1;
	}

	cx_sel_t cx_sel_entry = tsk->mcx_table[cx_sel];

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
		cx_map[cx_id].state_info[state_id].counter--;

		// clear the table
		tsk->mcx_table[cx_sel] = CX_INVALID_SELECTOR;

		// delete cx_sel_map_index from map
		// cx_map[cx_id].cx_sel_map_index[state_id] = -1;

		// Free from the OS
		kfree(tsk->cx_os_state_table[cx_sel].data);
		tsk->cx_os_state_table[cx_sel].ctx_status = 0;
		BUG_ON( cx_map[cx_id].state_info[state_id].counter < 0 );

		// let the state be used again
		if (cx_map[cx_id].state_info[state_id].counter == 0) {
			enqueue(cx_map[cx_id].avail_state_ids, state_id);
		}
		enqueue(tsk->cx_table_avail_indices, cx_sel);

	// Stateless cx's
	} else if (cx_map[cx_id].num_states == 0) {

		cx_map[cx_id].state_info[0].counter -= 1;
			
		// Don't clear the cx_selector_table entry unless the counter is at 0
		if (cx_map[cx_id].state_info[0].counter == 0) {
			tsk->mcx_table[cx_sel] = CX_INVALID_SELECTOR;

			enqueue(tsk->cx_table_avail_indices, cx_map[cx_id].state_info[0].index[0]);
			cx_map[cx_id].state_info[0].index[0] = -1;
		}

	} else {
		pr_info("made it to case that shouldn't happen\n");
		return -1; // Shouldn't make it to this case
	}

	return 0;
}

void exit_cx(struct task_struct *tsk) {
	
	// Free state
	if (tsk->mcx_table) {
		for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
			if (tsk->mcx_table[i] != CX_INVALID_SELECTOR) {
				pr_info("Freeing cx_index: %d\n", i);
				cx_close(tsk, i);
			}
		}
	}

	if (tsk->mcx_table)
		kfree(tsk->mcx_table);
	if (tsk->cx_os_state_table)
		kfree(tsk->cx_os_state_table);
	if (tsk->cx_table_avail_indices)
		free_queue(tsk->cx_table_avail_indices);
}

int initialize_state(uint status) 
{
        // 4. Read the state to get the state_size
        uint state_size = GET_CX_STATE_SIZE(status);

        if (state_size > 1023 || state_size < 0) {
                return 1;
        }

        // 5. Set the CXU to initial state
        uint sw_init = GET_CX_INITIALIZER(status);

        CX_WRITE_STATUS(CX_INITIAL);

        // hw required to set to dirty after init, while sw does it explicitly
        if (sw_init) {
                for (int i = 0; i < state_size; i++) {
                        CX_WRITE_STATE(i, 0);
                }
                CX_WRITE_STATUS(CX_DIRTY);
        }
        return 0;
}

void copy_state_to_os( uint state_size, uint index, struct task_struct *tsk ) 
{ 
        for (int i = 0; i < state_size; i++) {
                tsk->cx_os_state_table[index].data[i] = CX_READ_STATE(i);
        }
}

void copy_state_from_os( uint index, struct task_struct *tsk ) 
{
        cx_os_state_t src = tsk->cx_os_state_table[index];
        for (int i = 0; i < GET_CX_STATE_SIZE(src.ctx_status); i++) {
                CX_WRITE_STATE(i, src.data[i]);
        }
}

int cx_context_save(struct task_struct *tsk) {

		pr_info("save\n");

	    // Save the cx_index
        cx_sel_t cx_sel_index = cx_csr_read(CX_INDEX);
		// pr_info("cx_index: %d, %08x\n", cx_sel_index, &tsk->mcx_table[0]);
        tsk->cx_index = cx_sel_index;

		uint cx_error = cx_csr_read(CX_STATUS);
        tsk->cx_status = cx_error;

        for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
        
                // save the state data
                if (tsk->mcx_table[i] != CX_INVALID_SELECTOR) {

                        // write the index to be saved
                        cx_csr_write(CX_INDEX, i);
                        
                        uint cx_status = CX_READ_STATUS();

                        // Don't need to save data if it's not dirty
                        if (GET_CX_STATUS(cx_status) != CX_DIRTY) {
                                continue;
                        }
                        uint state_size = GET_CX_STATE_SIZE(cx_status);
                        
                        // copy state data to OS
                        copy_state_to_os(state_size, i, tsk);

                        // set the state context back to its initial state
                        int failure = initialize_state(cx_status);

                        if (failure) {
								pr_info("There was a failure!\n");
                                return -1;
                        }

                        // save the state context status
                        tsk->cx_os_state_table[i].ctx_status = cx_status;
                }
        }

        cx_csr_write(CX_INDEX, tsk->cx_index);
		pr_info("saved state\n");
        return 0;
}

int cx_context_restore(struct task_struct *tsk) {

		pr_info("restore\n");
	    // 1. Restore error
        csr_write(CX_STATUS, tsk->cx_status);
        
        // 2. Restore mcx_table
        csr_write(MCX_TABLE, &tsk->mcx_table[0]);

        // 3. Restore state information
        for (int i = 1; i < CX_SEL_TABLE_NUM_ENTRIES; i++) {
                
                cx_stctxs_t cx_stctxs = {.idx = tsk->cx_os_state_table[i].ctx_status};

                // ignore table indicies that aren't occupied, and only save dirty states
                if (tsk->mcx_table[i] != CX_INVALID_SELECTOR) {

                        // Write the index to be restored
                        cx_csr_write( CX_INDEX,  i );
                        
                        // Restore state
                        copy_state_from_os( i, tsk );

                        // Restoring status word + setting to clean
                        cx_stctxs.sel.cs = CX_CLEAN;
                        tsk->cx_os_state_table[i].ctx_status = cx_stctxs.idx;
                        CX_WRITE_STATUS(cx_stctxs.idx);
                }
        }

        // 4. Restore index
        cx_csr_write( CX_INDEX, tsk->cx_index );
		pr_info("restored state\n");
        return 0;
}