#include <stdint.h>
#include <stdio.h>

#include "../../include/cx-qemu/mcfu_select.h"
#include "../../zoo/addsub/addsub_func.h"
#include "../../zoo/muldiv/muldiv_func.h"
#include "../../include/parser.h"

typedef int64_t cx_guid_t;       // global: CX ID, a 128b GUID
typedef int32_t state_id_t;      // system: state index
typedef int32_t cx_id_t;

cx_config_info_t static cx_config_info;

void init_cx_map() {
    cx_config_info = read_files("");
}

int32_t mcfu_select_func(uint32_t mcfu_selector, int32_t cf_id, int32_t rs1, int32_t rs2) 
{
    cx_id_t cx_id = mcfu_selector & ~(~0 << 8);
    // cx_id_t cx_id = mcfu_selector & 0xff;

    // This should get the pointer table base address. Should get the function
    // pointer directly.
    // There should be bounds checking on cx_id, also bounds checking on the cf_id
    // to make sure that the functions exist. 
    // should be able to read a configuration file that builds this map dynamically.
    cx_guid_t cx_guid = cx_config_info.cx_config[cx_id].cx_guid;
    int32_t result = 0;
    if (cx_guid == CX_GUID_ADDSUB) {
        // TODO: this should be called via function pointers.
        // 1 map that looks up the cx_guid
        // 1 map that looks up the cf_id for a given cx_guid
        printf("Executing cx_guid addsub instructions\n");
        result = addsub_sel(cf_id, rs1, rs2);
    }
    else if (cx_guid == CX_GUID_MULDIV) {
        printf("Executing cx_guid muldiv instructions\n");
        result = muldiv_sel(cf_id, rs1, rs2);
    }
    else {
        printf("I'm sad :(\n");
    }
    return result;
}