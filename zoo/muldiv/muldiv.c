#include <stdint.h>

#include "muldiv.h"

__CX__ int32_t mul( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(0, a, b);
}

__CX__ int32_t div_( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(1, a, b);
}