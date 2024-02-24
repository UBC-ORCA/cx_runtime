#include <stdint.h>
#include <stdio.h>

#include "../../include/cx-qemu/mcfu_select.h"
#include "../../include/cx-qemu/addsub_func.h"

typedef int64_t cx_guid_t;       // global: CX ID, a 128b GUID
typedef int32_t state_id_t;      // system: state index
typedef int32_t cx_id_t;

typedef struct {
    cx_guid_t cx_guid;
    state_id_t state_id; // will at some point be # of state_ids, or a 
                         // list of available state_ids
} cx_map_t;

const cx_guid_t CX_GUID_B = 20;

cx_map_t static cx_map[2] = {
    {CX_GUID_ADDSUB, 0},
    {CX_GUID_B, 1}
};

int32_t mcfu_select_func(uint32_t mcfu_selector, int32_t cf_id, int32_t rs1, int32_t rs2) 
{
    cx_id_t cx_id = mcfu_selector & ~(~0 << 8);
    cx_guid_t cx_guid = cx_map[cx_id].cx_guid;
    int32_t result = 0;
    if (cx_guid == CX_GUID_ADDSUB) {
        printf("Executing cx_guid addsub instructions\n");
        result = addsub_sel(cf_id, rs1, rs2);
    } 
    else if (cx_guid == CX_GUID_B) {
        printf("Executing cx_guid B instructions\n");
    } 
    else {
        printf("I'm sad :(\n");
    }
    return result;
}