#include <stdint.h>

#include "addsub.h"    

__CX__ int32_t add( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(0, a, b);
}

__CX__ int32_t sub( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(1, a, b);
}

__CX__ int32_t add_1000( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(2, a, b);
}
