#include "ci.h" // CI Runtime: class use_ci { ... }

#define MUL 10;
#define CI_ID_MULTIPLY 11;

int main() 
{
    cxu_id_t CXU_ID_MULTIPLY = get_cxu(CI_ID_MULTIPLY);

    int32_t count = 0;
    int32_t a = 2;
    int32_t b = 3;

    // Should this be cx_cur_sel?
    if (alloc_sel_cxu(CXU_ID_MULTIPLY)) 
    {
        count = __builtin_riscv_cfu_reg(a, b, MUL); // rs1, rs2, cx_id
    }
    else
    {
        count = 2 * 3;
    }

    return 0;
}