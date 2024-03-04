#include <stdint.h>

#ifndef MCFU_SELECT_H
#define MCFU_SELECT_H

int32_t mcfu_select_func(uint32_t mcfu_selector, int32_t cf_id, int32_t rs1, int32_t rs2);

void init_cx_map(void);

#endif 