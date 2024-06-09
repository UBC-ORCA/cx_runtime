#include <stdint.h>

#include "mulacc.h"

__CX__ int32_t mac( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(0, a, b);
}

__CX__ int32_t reset( int32_t a, int32_t b ) 
{
    return CX_REG_HELPER(1, 0, 0);
}
