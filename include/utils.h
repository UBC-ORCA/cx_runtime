#ifndef UTILS_H
#define UTILS_H

// #include <stdlib.h>

#define MAX_CF_IDS 1024
#define MAX_CX_ID 255
#define NUM_CX 3

#define MAX_STATE_SIZE 1024 // number of words in a state

#define CX_ID_START_INDEX 0
#define CX_ID_BITS 8

#define CX_STATE_START_INDEX 16
#define CX_STATE_ID_BITS 8

#define CX_STATUS_START_INDEX 0
#define CX_STATUS_BITS 2

#define CX_STATE_SIZE_START_INDEX 2
#define CX_STATE_SIZE_BITS 10

#define CX_ERROR_START_INDEX 24
#define CX_ERROR_BITS 8

// ========= cx helpers ===========

#define GET_BITS(cx_sel, start_bit, n) \
    (cx_sel >> start_bit) & (((1 << n) - 1) )

// ========= cx selector helpers ===========

#define GET_CX_ID(cx_sel) \
    GET_BITS(cx_sel, CX_ID_START_INDEX, CX_ID_BITS)

#define GET_CX_STATE(cx_sel) \
    GET_BITS(cx_sel, CX_STATE_START_INDEX, CX_STATE_ID_BITS)

// ========= cx state context status helpers ===========

#define GET_CX_STATUS(cx_sel) \
    GET_BITS(cx_sel, CX_STATUS_START_INDEX, CX_STATUS_BITS)

#define GET_CX_STATE_SIZE(cx_sel) \
    GET_BITS(cx_sel, CX_STATE_SIZE_START_INDEX, CX_STATE_SIZE_BITS)

#define GET_CX_ERROR(cx_sel) \
    GET_BITS(cx_sel, CX_ERROR_START_INDEX, CX_ERROR_BITS)

typedef unsigned int uint;

typedef union {
     struct {
        uint cs     : CX_STATUS_BITS;
        uint state_size : CX_STATE_SIZE_BITS;
        uint reserved0  : 12;
        uint error      : CX_ERROR_BITS;
     } sel;
      uint idx;
} cx_stctxs_t;

enum {
    OFF, 
    INITIAL, 
    CLEAN,
    DIRTY
};       

#define CX_REG_HELPER(cf_id, result, rs1, rs2)      ({   \
    asm volatile("      cx_reg " #cf_id ",%0,%1,%2;\n\t" \
                 : "=r" (result)                         \
                 : "r" (rs1), "r" (rs2)                  \
                 :                                       \
    );   }) 


#define CX_WRITE_STATE(rs1, rs2)  ({                   \
    asm volatile("      cx_reg 1020, zero,%0,%1;\n\t"  \
                 :                                     \
                 : "r" (rs1), "r" (rs2)                \
                 :                                     \
    );   }) 

#define CX_WRITE_STATUS(rs1)  ({                        \
    asm volatile("      cx_reg 1022, x0,%0,x0;\n\t"     \
                 :                                      \
                 : "r" (rs1)                            \
                 :                                      \
    );   }) 


#define CX_REG(cf_id, rs1, rs2)                         \
    int32_t result = -1;                                \
    CX_REG_HELPER(cf_id, result, rs1, rs2);             \
    return result

#define CX_READ_STATUS(dest)                CX_REG_HELPER(1023, dest, 0, 0)
// #define CX_WRITE_STATUS(status)             CX_REG_HELPER(1022, 0, status, 0)
#define CX_READ_STATE(dest, index)          CX_REG_HELPER(1021, dest, index, 0)
// #define CX_WRITE_STATE(index, value)        CX_REG_HELPER(1020, 0, index, value)

#endif