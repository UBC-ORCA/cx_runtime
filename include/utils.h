#ifndef UTILS_H
#define UTILS_H

#define MCX_SELECTOR  0x012 // should be 0xBC0
#define CX_INDEX      0x011 // should be 0x800
#define CX_STATUS     0x801 // should be 0x801, was using 0x013
#define MCX_TABLE     0x145 // should be 0xBC1

#define CUSTOM0 0xb
#define CUSTOM1 0x2b
#define CUSTOM2 0x5b

#define CX_REG_TYPE  CUSTOM0
#define CX_IMM_TYPE  CUSTOM1
#define CX_FLEX_TYPE CUSTOM2

#define CX_INVALID_SELECTOR 0x10000000
#define CX_LEGACY 0
#define MCX_VERSION 1
#define CX_SEL_TABLE_NUM_ENTRIES 1024
// #define DEBUG 0
#define UNASSIGNED_STATE -1

#define MAX_CF_IDS 1024
#define MAX_CX_ID 255
#define NUM_CX 3

#define MAX_CXU_ID 1 << CX_ID_BITS
#define MAX_STATE_ID 1 << STATE_ID_BITS

#define MAX_STATE_SIZE 1024 // number of words in a state

/* cx_selector CSR */
#define CX_ID_START_INDEX 0
#define CX_ID_BITS 8

#define CX_STATE_START_INDEX 16
#define CX_STATE_ID_BITS 8

#define CX_CXE_START_INDEX 28
#define CX_CXE_BITS 1

#define CX_VERSION_START_INDEX 29
#define CX_VERSION_ID_BITS 3

/* cx_context_status_word CSR */
#define CX_STATUS_START_INDEX 0
#define CX_STATUS_BITS 2

#define CX_STATE_SIZE_START_INDEX 3
#define CX_STATE_SIZE_BITS 10

#define CX_ERROR_START_INDEX 24
#define CX_ERROR_BITS 8

#define CX_INITIALIZER_START_INDEX 2
#define CX_INITIALIZER_BITS 1

#define CX_HW_INIT 0
#define CX_OS_INIT 1

/* cx_status CSR */
#define CX_IV_START_INDEX 0
#define CX_IV_BITS 1

#define CX_IC_START_INDEX 1
#define CX_IC_BITS 1

#define CX_IS_START_INDEX 2
#define CX_IS_BITS 1

#define CX_OF_START_INDEX 3
#define CX_OF_BITS 1

#define CX_IF_START_INDEX 4
#define CX_IF_BITS 1

#define CX_OP_START_INDEX 5
#define CX_OP_BITS 1

#define CX_CU_START_INDEX 6
#define CX_CU_BITS 1

/* cx share type */
#define CX_SHARE_START_INDEX 0
#define CX_SHARE_BITS 2

// ========= cx helpers ===========

#define GET_BITS(cx_sel, start_bit, n) \
    ((cx_sel >> start_bit) & (((1 << n) - 1) ))

// ========= cx selector helpers ===========

#define GET_CX_ID(cx_sel) \
    GET_BITS(cx_sel, CX_ID_START_INDEX, CX_ID_BITS)

#define GET_CX_STATE(cx_sel) \
    GET_BITS(cx_sel, CX_STATE_START_INDEX, CX_STATE_ID_BITS)

#define GET_CX_CXE(cx_sel) \
    GET_BITS(cx_sel, CX_CXE_START_INDEX, CX_CXE_BITS)

#define GET_CX_VERSION(cx_sel) \
    GET_BITS(cx_sel, CX_VERSION_START_INDEX, CX_VERSION_ID_BITS)

// ========= cx state context status helpers ===========

#define GET_CX_STATUS(cx_sel) \
    GET_BITS(cx_sel, CX_STATUS_START_INDEX, CX_STATUS_BITS)

#define GET_CX_INITIALIZER(cx_sel) \
    GET_BITS(cx_sel, CX_INITIALIZER_START_INDEX, CX_INITIALIZER_BITS)

#define GET_CX_STATE_SIZE(cx_sel) \
    GET_BITS(cx_sel, CX_STATE_SIZE_START_INDEX, CX_STATE_SIZE_BITS)

#define GET_CX_ERROR(cx_sel) \
    GET_BITS(cx_sel, CX_ERROR_START_INDEX, CX_ERROR_BITS)

// ========= cx status helpers ===========



// ========= cx share type helpers ==========
#define GET_SHARE_TYPE(cx_share) \
    GET_BITS(cx_share, CX_SHARE_START_INDEX, CX_SHARE_BITS) 


typedef unsigned int uint;

typedef union {
     struct {
        uint cs          : CX_STATUS_BITS;
        uint initializer : CX_INITIALIZER_BITS;
        uint state_size  : CX_STATE_SIZE_BITS;
        uint reserved0   : 11;
        uint error       : CX_ERROR_BITS;
     } sel;
      uint idx;
} cx_stctxs_t;

typedef union {
    struct {
        uint cx_id     : CX_ID_BITS;
        uint reserved1 : 8;
        uint state_id  : CX_STATE_ID_BITS;
        uint reserved0 : 4;
        uint cxe       : CX_CXE_BITS;
        uint version   : CX_VERSION_ID_BITS;
    } sel;
        uint idx;
 } cx_selidx_t;

typedef union {
    struct {
        uint IV        : CX_IV_BITS;
        uint IC        : CX_IC_BITS;
        uint IS        : CX_IS_BITS;
        uint OF        : CX_OF_BITS;
        uint IF        : CX_IF_BITS;
        uint OP        : CX_OP_BITS;
        uint CU        : CX_CU_BITS;
        uint reserved0 : 25;
    } sel;
    int idx;
} cx_status_t;

enum CX_CS {
    OFF, 
    INITIAL, 
    CLEAN,
    DIRTY
};

enum SHARE_TYPE {
	EXCLUDED,
	PROCESS_SHARED,
	GLOBAL_SHARED
};

#define CX_REG_HELPER(cf_id, rs1, rs2)      ({           \
    register int __v;                                    \
    asm volatile("      cx_reg " #cf_id ",%0,%1,%2;\n\t" \
                 : "=r" (__v)                            \
                 : "r" (rs1), "r" (rs2)                  \
                 : "memory"                              \
    );                                                   \
	__v;							                     \
    })

#define __ASM_STR(x)	#x
#define cx_csr_read(csr)				                \
({								                        \
	register unsigned int __v;				            \
	__asm__ __volatile__ ("csrr %0, " __ASM_STR(csr)	\
			      : "=r" (__v) :			            \
			      : "memory");			                \
	__v;							                    \
})

#define cx_csr_write(csr, val)					        \
({								                        \
	unsigned int __v = (unsigned int)(val);		        \
	__asm__ __volatile__ ("csrw " __ASM_STR(csr) ", %0"	\
			      : : "rK" (__v)			            \
			      : "memory");			                \
})

#define CX_WRITE_STATE(index, value)  ({                   \
    asm volatile("      cx_reg 1020, zero,%0,%1;\n\t"      \
                 :                                         \
                 : "r" (index), "r" (value)                \
                 :                                         \
    );   })

#define CX_WRITE_STATUS(status)  ({                        \
    asm volatile("      cx_reg 1022, zero,%0,x0;\n\t"      \
                 :                                         \
                 : "r" (status)                            \
                 :                                         \
    );   })

#define CX_READ_STATUS()                CX_REG_HELPER(1023, 0, 0)
// #define CX_WRITE_STATUS(status)             CX_REG_HELPER(1022, 0, status, 0)
#define CX_READ_STATE(index)            CX_REG_HELPER(1021, index, 0)
// #define CX_WRITE_STATE(index, value)        CX_REG_HELPER(1020, 0, index, value)

#endif