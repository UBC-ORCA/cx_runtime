#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/syscalls.h>


typedef union {
     struct {
        uint cx_id     : 8;
        uint reserved1 : 8;
        uint state_id  : 8;
        uint reserved0 : 4;
        uint version   : 4;
     } sel;
      int idx;
 } cx_selidx_t;

#define CX_VERSION 1
#define CX_STATE_AVAIL 1
#define CX_STATE_UNAVAIL 0

int get_free_state(int cx_id) {
        for (int i = 0; i < CX_NUM_STATES; i++) {
                if (current->cx_map[cx_id].avail_state_ids[i]) {
                        current->cx_map[cx_id].avail_state_ids[i] = CX_STATE_UNAVAIL;
                        return i;
                }
        }
        return -1;
}

SYSCALL_DEFINE1(cx_open, int, cx_guid)
{
        static int i = 0;
        int cx_id = -1;

        for (int j = 0; j < NUM_CX; j++) {
                if (current->cx_map[j].cx_guid == cx_guid) {
                        cx_id = j;
                }
        }

        if (cx_id == -1) {
                return -1;
        }

        int state_id = get_free_state(cx_id);

        cx_selidx_t cx_sel = {.sel = {.cx_id = cx_id, 
                                  .state_id = state_id,
                                  .version = CX_VERSION}};

        current->mcx_table[i] = cx_sel.idx;

        return i++;
}

// Temporary, will define this elsewhere later
SYSCALL_DEFINE0(cx_init)
{
        current->mcx_table = kzalloc(sizeof(int) * 1024, GFP_KERNEL);
        if (!current->mcx_table) {
                // some sort of allocation error here
                return -1;
        }
        asm volatile (
                "csrw 0x145, %0        \n\t" // TODO: 145 is temporary - will be 0xBC1 when QEMU works
                :
                : "r" (&current->mcx_table[0])
                :
                );
        
        current->cx_map[0].cx_guid = 11;
        current->cx_map[1].cx_guid = 4;

        current->cx_map[0].num_states = CX_NUM_STATES;
        current->cx_map[1].num_states = CX_NUM_STATES;
        
        for (int i = 0; i < NUM_CX; i++) {
                for (int j = 0; j < CX_NUM_STATES; j++) {
                        current->cx_map[i].avail_state_ids[j] = CX_STATE_AVAIL;
                }
        }
        return 0;
}