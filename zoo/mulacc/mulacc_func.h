#include <stdint.h>

#ifndef MULACC_FUNC_H
#define MULACC_FUNC_H

#define CX_MULACC_NUM_FUNCS 2
#define CX_MULACC_NUM_STATES 2

extern int32_t (*cx_func_mulacc[]) ( int32_t, int32_t, int32_t );

#endif
