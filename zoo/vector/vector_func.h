#include <stdint.h>
#include "vector_common.h"

#ifndef VECTOR_FUNC_H
#define VECTOR_FUNC_H

extern int32_t (*cx_func_vector[]) ( int32_t, int32_t, int32_t );

void init_cx_func_vector( void );

#endif