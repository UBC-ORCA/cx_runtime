#include <stdint.h>

#include "../include/muldiv.h"

__CX__ int32_t mul( int32_t a, int32_t b ) 
{
    CX_REG(0, a, b);
}

__CX__ int32_t div_( int32_t a, int32_t b ) 
{
    CX_REG(1, a, b);
}