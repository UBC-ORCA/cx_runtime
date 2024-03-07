#include <stdint.h>

#include "addsub.h"    

__CX__ int32_t add( int32_t a, int32_t b ) 
{
    CX_REG(0, a, b);
}

__CX__ int32_t sub( int32_t a, int32_t b ) 
{
    CX_REG(1, a, b);
}