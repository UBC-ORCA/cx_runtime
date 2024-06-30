#include <stdint.h>

#include "p-ext_func.h"

#define XLEN 32

#define i8 int8_t
#define u8 uint8_t

#define i16 int16_t
#define u16 uint16_t

#define i32 int32_t
#define u32 uint32_t

#define i64 int64_t
#define u64 uint64_t

#define int i32
#define uint u32

#define GET_BITS(reg, start_bit, n) \
    ((reg >> start_bit) & (((1 << n) - 1) ))

#define GET_BITS_U(reg, start_bit, n) \
    ((uint)GET_BITS(reg, start_bit, n))

static inline i32 add16_func(i32 a, i32 b, int32_t state_id)
{
    uint result = 0, mask = 0xFFFF;
    i16 sum, rs1_h, rs2_h;
    for (int i = 0; i < (XLEN / 16); i++) {
        rs1_h = GET_BITS(a, i * 16, 16);
        rs2_h = GET_BITS(b, i * 16, 16);
        sum = rs1_h + rs2_h;
        result |= (sum << i * 16) & mask;
        mask <<= 16;
    }
    return result;
}

static inline i32 sra16_func(i32 a, i32 sa, int32_t state_id)
{
    uint result = 0, mask = 0xFFFF;
    i16 rs1_h;
    for (int i = 0; i < (XLEN / 16); i++) {
        rs1_h = GET_BITS(a, i * 16, 16);
        sa &= 0xFF; // get lower 8 bits
        i16 res = rs1_h >> sa;
        result |= (res << i * 16) & mask;
        mask <<= 16;
    }
    return result;
}

static inline i32 smul16_func(i32 a, i32 b, int32_t state_id)
{
    uint result = 0, mask = 0xFFFF;
    i16 sum, rs1_h, rs2_h;
    for (int i = 0; i < (XLEN / 16); i++) {
        rs1_h = GET_BITS(a, i * 16, 16);
        rs2_h = GET_BITS(b, i * 16, 16);
        sum = rs1_h * rs2_h;
        result |= (sum << i * 16) & mask;
        mask <<= 16;
    }
    return result;
}

static inline i32 add8_func(i32 a, i32 b, int32_t state_id)
{
    uint result = 0, mask = 0xFF;
    i8 sum, rs1_h, rs2_h;
    for (int i = 0; i < (XLEN / 8); i++) {
        rs1_h = GET_BITS_U(a, i * 8, 8);
        rs2_h = GET_BITS_U(b, i * 8, 8);
        sum = rs1_h + rs2_h;
        result |= (sum << i * 8) & mask;
        mask <<= 8;
    }
    return result;
}

static inline i32 sra8_func(i32 a, i32 sa, int32_t state_id)
{
    uint result = 0, mask = 0xFF;
    int sum;
    i8 rs1_h;
    for (int i = 0; i < (XLEN / 8); i++) {
        rs1_h = GET_BITS(a, i * 8, 8);
        sa &= 0xF; // get lower 4 bits
        i8 res = rs1_h >> sa;
        result |= (res << i * 8) & mask;
        mask <<= 8;
    }
    return result;
}

static inline i32 smul8_func(i32 a, i32 b, int32_t state_id)
{
    uint result = 0, mask = 0xFF;
    i8 sum, rs1_h, rs2_h;
    for (int i = 0; i < (XLEN / 8); i++) {
        rs1_h = GET_BITS(a, i * 8, 8);
        rs2_h = GET_BITS(b, i * 8, 8);
        sum = rs1_h * rs2_h;
        result |= (sum << i * 8) & mask;
        mask <<= 8;
    }
    return result;
}

int32_t (*cx_func_pext[]) (int32_t, int32_t, int32_t) = {
    add16_func,
    sra16_func,
    smul16_func,
    add8_func,
    sra8_func,
    smul8_func
};