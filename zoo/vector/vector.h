#include <stdint.h>
#include "../../include/ci.h"
#include "vector_common.h"

#ifndef VECTOR_H
#define VECTOR_H

__CX__ int32_t addv( int32_t a, int32_t b );
__CX__ int32_t set_inc( int32_t reg );
__CX__ int32_t set2( int32_t reg );
__CX__ int32_t resetv( void );

#endif