#include <stdint.h>

#ifndef CI_H
#define CI_H

// TYPDEFS

typedef int64_t cx_guid_t;       // global: CX ID, a 128b GUID
typedef int32_t cxu_guid_t;      // cxu package global name
typedef int32_t cx_id_t;         // system: CXU index

typedef int32_t state_id_t;      // system: state index
typedef int32_t cx_share_t;      // context sharing permissions

typedef int32_t cx_sel_t;        // hart: CX selector (value (No CX Table) or index
                                 //       (when there is a CX Table))

typedef int32_t cxu_state_context_status_t; // per state    


// ABI

#define __CX__         //(FIXME: compiler builtin)
#define CX_LEGACY 0


// MACROS

#define CFU_REG(rd, cf_id, rs1, rs2) \
    asm volatile("      cfu_reg " #cf_id ",%0,%1,%2;\n" \
                 : "=r" (rd)             \
                 : "r" (rs1), "r" (rs2)  \
                 :                       \
    )


// API

extern static void cx_init();
cx_sel_t cx_open(cx_guid_t cx_guid, cx_share_t cx_share);
cx_sel_t cx_select(cx_sel_t cx_sel);
void     cx_close(cx_sel_t cx_sel);
void cx_deselect_and_close(cx_sel_t cx_sel);

#endif