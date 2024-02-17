#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "ci.h"

int main() 
{
    int32_t a = 3;
    int32_t b = 5;
    int32_t cf_id = 2;
    int32_t c = cfu_reg(a, b, cf_id);
    printf("c: %d\n", c);
    return 0;
}