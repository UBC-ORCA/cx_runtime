#include <stdint.h>

#ifndef CI_H
#define CI_H

typedef int64_t cx_guid_t;       // global: CX ID, a 128b GUID
typedef int32_t cx_id_t;         // system: CXU index
typedef int32_t state_id_t;      // system: state index
typedef int32_t cx_sel_t;        // hart: CX selector (value or index)
typedef int32_t cx_share_t;      // context sharing permissions
typedef int32_t cxu_guid_t;      // cxu package global name

extern const cx_guid_t CX_GUID_A;
extern const cx_guid_t CX_GUID_B;

void init_cfu_runtime();

int32_t cfu_reg(int32_t, int32_t, int32_t);

// standard cx runtime calls
cx_sel_t cx_open(cx_guid_t, cx_share_t);

cx_sel_t cx_select(cx_sel_t);

void cx_close(cx_sel_t);

// Bundled function
void cx_deselect_and_close(cx_sel_t);

#endif