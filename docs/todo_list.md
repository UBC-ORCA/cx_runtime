# Short Term Objectives

:x: Move static part of cx_map in a device struct, similar to a PCIe device

:x: Trap on first use of cx_{imm, reg, flex} for virtualization of threads, allowing state sharing via loading / storing to / from memory

:x: Fix builds

:x: Raise mtval CSR when we have an invalid cx_selector

:x: Support saving cxu state data thread-specific instead of process-specific

:x: *(Partially completed) Create CX_STATUS CSR to keep track of errors (%2.2.2) 

:x: Document the functionality of cx_{open, close, sel}

:x: Modify QEMU so that the proper CSR addresses can be used

:x: Check objdump to make sure that inlined functions are actually being inlined

:x: replace instances of setting errno with first getting the local errno with `__errno_location(void)`.

# Medium Term Objectives

:x: Create a CI / CD test harness

:x: Add `probe` operation to check if a given cx can properly be accessed

:x: Consider how the user will interact with the `cx_error();` function - when will it be called? 
    should it be a part of `cx_close();`?

# Completed objectives

:white_check_mark: Have tests for m_mode ci.h (ci_m.h)

:white_check_mark: Add tests, making sure that the correct state off / init / dirty / clean values are set correctly

:white_check_mark: Be able to save + load contexts

:white_check_mark: CX_Error is used in examples as a fence instruction before issuing another cx_sel instruction

:white_check_mark: Fix m mode cx_{open, close, sel}, so that it works the same as u mode

:white_check_mark: Fix stateless cxs

:white_check_mark: Move structs in cfu_helper to be initialized elsewhere + add padding

:white_check_mark: Define error codes, and use perror instead of printf

:white_check_mark: Move cx's to zoo directory

:white_check_mark: Remove mcx_selector in favor of arrays of function pointers

:white_check_mark: rename instances of cfu to cx

:white_check_mark: Add checks to cx_open / cx_close for robustness in multi-threaded scenarios
