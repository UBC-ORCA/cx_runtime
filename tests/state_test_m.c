#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "../../../../research/riscv-tools/cx_runtime/include/ci.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/mulacc/mulacc.h"

#define CX_SEL_TABLE_NUM_ENTRIES 1024

static cx_stctxs_t expected_stctxs = {.sel = {
                                .dc = CX_DIRTY,
                                .R = 0,
                                .state_size = 1,
                                .version = 1
                              }};

void state_test() {
    int a = 3;
    int b = 5;
    int result;

    cx_share_t share_A = 0, share_C = 0;
    cx_sel_t mcx_selector;

    /* cx_error should be set to 0 initially */
    uint cx_error = cx_error_read();
    assert ( cx_error == 0 );

    int cx_sel_A0 = cx_open(CX_GUID_MULACC, share_A, -1);
    assert ( cx_sel_A0 == 0x20000002 );

    cx_error_clear();
    cx_sel(cx_sel_A0);

    uint status = CX_READ_STATUS();
    
    assert (status == expected_stctxs.idx);

    result = mac(a, b);
    assert( result == 15 );

    status = CX_READ_STATUS();

    assert (status == expected_stctxs.idx);

    cx_error = cx_error_read();
    assert ( cx_error == 0 );

    cx_close(cx_sel_A0);

    /* Testing multiple states */
    int cx_sel_A1 = cx_open(CX_GUID_MULACC, share_A, -1);
    int cx_sel_A2 = cx_open(CX_GUID_MULACC, share_A, -1);
    assert( cx_sel_A1 == 0x20000002 );
    assert( cx_sel_A2 == 0x20010002 );

    cx_error_clear();
    cx_sel(cx_sel_A1);

    result = mac(a, a);
    assert( result == 9 );
    cx_error = cx_error_read();
    assert ( cx_error == 0 );

    cx_error_clear();
    cx_sel(cx_sel_A2);
    result = mac(b, b);
    assert( result == 25 );
    cx_error = cx_error_read();
    assert ( cx_error == 0 );

    cx_close(cx_sel_A1);

    uint cx_sel_test = -1;

    const int INVALID_CX_GUID = 0;
    int cx_sel_invalid = cx_open(INVALID_CX_GUID, share_A, -1);
    
    assert( cx_sel_invalid == -1 );
    cx_sel( CX_LEGACY );
}

int main() {
    cx_init();
    cx_sel( CX_LEGACY );
    state_test();
    printf("state test passed (m mode)\n");
    return 0;
}
