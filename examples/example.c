#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/ci.h"
#include "../zoo/addsub/addsub.h"
#include "../zoo/muldiv/muldiv.h"

// definitions for cx_id's are in here
#include "../include/parser.h"


cx_sel_t test_cx_open( cx_guid_t my_guid, cx_share_t my_share )
{
    cx_sel_t my_sel = cx_open( my_guid, my_share );
    if( my_sel >= 0 ) return my_sel;

    fprintf( stderr, "error: cx_open guid %08x share %d\n", my_guid, my_share );
    exit( -1 );
}

void my_cx_test() 
{
    int32_t a = 3;
    int32_t b = 5;
    
    int32_t result = 0;

    cx_share_t my_shareA=0, my_shareB=0;

    cx_sel_t my_selA = test_cx_open( CX_GUID_ADDSUB, my_shareA );
    cx_sel_t my_selB = test_cx_open( CX_GUID_MULDIV, my_shareB );

    cx_select( my_selA );  // ensure my_selA is in correct range
    // cx_error(); should look @ cx status register, and see which (if any)
    //             errors are active
    //             ABI issue: when do we write / clear error status?
    // cx_csr_rw(); // wraps cx_csr r/w
    // 
    result = add( a, b ); // ABI rule 6
    printf( "result add: %d\n", result );

    cx_select( my_selB );
    result = mul( a, b ); 
    printf( "result mul: %d\n", result );

    cx_select( my_selA );
    result = sub( a, b );
    printf( "result sub: %d\n", result );

    result = add_1000( a, b );
    printf( "result add_1000: %d\n", result );

    cx_close( my_selB );
    cx_close( my_selA );

    cx_select( CX_LEGACY ); // ABI rule 4
}


int main()
{
    /* RISCV gcc does not support dirnet.h */
    int32_t const num_cxs = 2;
    char *cx_yamls[2] = {"/home/bf/research/riscv-tools/cx_runtime/zoo/addsub/addsub.yaml", 
                         "/home/bf/research/riscv-tools/cx_runtime/zoo/muldiv/muldiv.yaml"};
    cx_init(cx_yamls, num_cxs);
    cx_select( CX_LEGACY ); // ABI rule 1
    my_cx_test();
    return 0;
}