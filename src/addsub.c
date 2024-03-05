#include <stdint.h>

#include "../include/addsub.h"

__CX__ int32_t add( int32_t a, int32_t b ) 
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

__CX__ int32_t sub( int32_t a, int32_t b ) 
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