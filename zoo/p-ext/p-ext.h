#include <stdint.h>
#include "../../include/ci.h"
#include "p-ext_common.h"

#ifndef PEXT_H
#define PEXT_H

__CX__ int32_t add16( int32_t a, int32_t b );
__CX__ int32_t sra16( int32_t a, int32_t b );
__CX__ int32_t smul16( int32_t a, int32_t b );
__CX__ int32_t add8( int32_t a, int32_t b );
__CX__ int32_t sra8( int32_t a, int32_t b );
__CX__ int32_t smul8( int32_t a, int32_t b );

#endif