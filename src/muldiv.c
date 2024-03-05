#include <stdint.h>

#include "../include/muldiv.h"

__CX__ int32_t mul( int32_t a, int32_t b ) 
{
    int32_t result = 0;
    asm volatile(
         "cfu_reg 0,%0,%1,%2;\n"
        : "=r" (result)     
        : "r" (a), "r" (b)
        :
    );
    return result;
}

__CX__ int32_t div_( int32_t a, int32_t b ) 
{
    int32_t result = 0;
    asm volatile(
         "cfu_reg 1,%0,%1,%2;\n"
        : "=r" (result)       
        : "r" (a), "r" (b)
        :
    );
    return result;
}