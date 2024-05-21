#include <stdint.h>

#ifndef MULDIV_FUNC_H
#define MULDIV_FUNC_H

#define CX_MULDIV_NUM_FUNCS 2

extern int32_t (*cx_func_muldiv[]) ( int32_t, int32_t, int32_t );

#endif
