#include <stdint.h>
#include "nn_acc_common.h"
#include "../../include/utils.h"

#ifndef NN_ACC_FUNC_H
#define NN_ACC_FUNC_H

extern int32_t (*cx_func_nn_acc[]) ( int32_t, int32_t, cx_selidx_t );

void init_cx_func_nn_acc( void );

#endif
