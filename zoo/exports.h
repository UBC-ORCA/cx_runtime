#ifndef EXPORTS_H
#define EXPORTS_H

#include "addsub/addsub_func.h"
#include "muldiv/muldiv_func.h"

extern cx_func_stub_t[MAX_CX_ID+1] = {
    cx_func_addsub,
    cx_func_muldiv,
    cx_func_err, 
};

#endif // EXPORTS_H