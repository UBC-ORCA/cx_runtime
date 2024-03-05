#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/ci.h"
#include "../include/addsub.h"
#include "../include/muldiv.h"

// definitions for cx_id's are in here
#include "../include/parser.h"


cx_sel_t test_cx_open( my_cxguid_t my_guid, cx_share_t my_share )
{
    cx_sel_t my_sel = cx_open( my_guid, my_share );
    if( my_sel >= 0 ) return my_sel;

    fprintf( stderr, "error: cx_open guid %08x share %d\n", my_guid, my_share );
    exit( -1 );
}


// FIXME:
// Please move the files defining add()/sub() and mul()/div_() into two distinct CX
// libraries (ie, their own folders with identical internal organization). Right now
// they are organized into the Runtime code base without distinction. They don't need
// to be in separate repos; create a top-level folder called "zoo", eg:
// https://github.com/grayresearch/CX/tree/main/zoo


void basic_cx_test() 
{
    int32_t a = 3;
    int32_t b = 5;
    
    int32_t result = 0;

    cx_share_t my_shareA=0, my_shareB=0;
    cx_sel_t prev_sel;

    cx_sel_t my_selA = test_cx_open( CX_GUID_ADDSUB, my_shareA );
    cx_sel_t my_selB = test_cx_open( CX_GUID_MULDIV, my_shareB );

    prev_sel = cx_select( my_selA );  // ensure my_selA is in correct range
    result = add( a, b ); // new ABI requirement: decorated function doesn't require caller to set Legacy
    printf( "result add: %d\n", result );

    prev_sel = cx_select( my_selB );
    result = mul( a, b ); 
    printf( "result mul: %d\n", result );

    prev_sel = cx_select( my_selA );
    result = sub( a, b );
    printf( "result sub: %d\n", result );

    cx_close( my_selB );
    cx_close( my_selA );

    cx_select( CX_LEGACY ); // new ABI requirement: if cx_sel changed in this function, restore Legacy on exit
}


int main()
{
    cx_init();
    cx_select( CX_LEGACY ); // new ABI requirement: non-decorated functions require caller to set Legacy
    my_cx_test();
    return 0;
}