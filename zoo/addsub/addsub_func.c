#include <stdint.h>

#include "addsub_func.h"

static inline int32_t add_func(int32_t a, int32_t b)
{
    return a + b;
}

static inline int32_t sub_func(int32_t a, int32_t b)
{
    return a - b;
}

static inline int32_t add_1000(int32_t a, int32_t b)
{
    return a + b + 1000;
}

int32_t (*addsub_func[]) (int32_t, int32_t) = {
    add_func,
    sub_func,
    add_1000
};
