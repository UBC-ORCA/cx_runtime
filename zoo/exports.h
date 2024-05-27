#ifndef EXPORTS_H
#define EXPORTS_H

#include <stdint.h>

#include "../include/utils.h"
#include "addsub/addsub_func.h"
#include "muldiv/muldiv_func.h"
#include "mulacc/mulacc_func.h"

#define CX_ERROR_NUM_FUNCS 0

typedef int32_t (*(*cx_func_stub_t)) (int32_t, int32_t, int32_t);

extern int32_t (*cx_func_error[]) (int32_t, int32_t, int32_t);

extern cx_func_stub_t cx_funcs[MAX_CX_ID];

extern int32_t num_cfs[MAX_CX_ID];

void cx_init_funcs( void );

#endif // EXPORTS_H