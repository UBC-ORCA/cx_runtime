#include <stdint.h>

#include "muldiv_func.h"

static inline int32_t mul_func(int32_t a, int32_t b)
{
    return a * b;
}

static inline int32_t div_func(int32_t a, int32_t b)
{
    return a / b;
}

int32_t (*muldiv_func[]) (int32_t, int32_t) = {
    mul_func,
    div_func
};
