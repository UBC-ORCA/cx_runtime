#include <stdint.h>
#include "../../include/ci.h"
#include "nn_acc_common.h"

#ifndef NN_ACC_H
#define NN_ACC_H

__CX__ static inline int32_t nn_acc( int32_t a, int32_t b ) {
    return CX_REG_HELPER(0, a, b);
}; 
__CX__ static inline int32_t nn_reset( void ) {
    return CX_REG_HELPER(1, 0, 0);
};
__CX__ static inline int32_t nn_relu( void ) {
    return CX_REG_HELPER(2, 0, 0);
};

#endif