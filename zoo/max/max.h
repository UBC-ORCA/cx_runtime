#include <stdint.h>
#include "../../include/ci.h"
#include "max_common.h"

#ifndef MAX_H
#define MAX_H

__CX__ static inline int max(int a, int b) {
    return CX_REG_HELPER(0, a, b);
};

#endif