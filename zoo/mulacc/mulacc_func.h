#include <stdint.h>
#include "mulacc_common.h"

#ifndef MULACC_FUNC_H
#define MULACC_FUNC_H

extern int32_t (*cx_func_mulacc[]) ( int32_t, int32_t, int32_t );

void init_cx_func_mulacc( void );

#endif
