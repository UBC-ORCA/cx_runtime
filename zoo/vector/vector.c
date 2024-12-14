#include <stdint.h>

#include "vector.h"

__CX__ int32_t addv( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(0, a, b);
}

__CX__ int32_t set_inc( int32_t reg ) 
{
    return CX_REG_HELPER(1, reg, 0);
}

__CX__ int32_t set2( int32_t reg ) 
{
    return CX_REG_HELPER(2, reg, 0);
}

__CX__ int32_t resetv( void ) 
{
    return CX_REG_HELPER(3, 0, 0);
}