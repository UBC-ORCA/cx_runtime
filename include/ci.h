#include <stdint.h>
#include "utils.h"

#ifndef CI_H
#define CI_H

// ABI

// TYPDEFS

typedef int32_t cx_guid_t;       // global: CX ID, a 128b GUID
typedef int32_t cxu_guid_t;      // cxu package global name
typedef int32_t cx_id_t;         // system: CXU index

typedef int32_t state_id_t;      // system: state index
typedef int32_t cx_share_t;      // context sharing permissions
typedef int32_t cx_virt_t;       // context virtualization permissions

typedef int32_t cx_sel_t;        // hart: CX selector (value (No CX Table) or index
                                 //       (when there is a CX Table))

typedef uint32_t cx_error_t;     //

typedef int32_t cxu_sctx_t;      // per state

// MACROS

// TODO: RESULT

// API

void        cx_init(void);
cx_sel_t    cx_open(cx_guid_t cx_guid, cx_share_t cx_share, cx_sel_t cx_sel);
void        cx_close(cx_sel_t cx_sel);
cx_error_t  cx_error_read(void);
void        cx_error_clear(void);
void        cx_deselect_and_close(cx_sel_t cx_sel);

static void inline cx_sel(cx_sel_t cx_sel) {
    cx_csr_write(CX_INDEX, cx_sel);
};

#endif