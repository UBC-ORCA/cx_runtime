# Short Term Objectives

:x: Create CX_STATUS CSR to keep track of errors (%2.2.2) [Needs spike CF_ID errors to be handled]

:x: Check objdump to make sure that inlined functions are actually being inlined

# Medium Term Objectives

:x: Trap on write to mcx_selector, so that it writes to the correct address (0xBC0 instead of 0x802)

:x: Add `probe` operation to check if a given cx can properly be accessed

:x: Create a CI / CD test harness

:x: Consider how the user will interact with the `cx_error();` function - when will it be called? 
    should it be a part of `cx_close();`?

# Completed objectives

:white_check_mark: Move structs in cfu_helper to be initialized elsewhere + add padding

:white_check_mark: Define error codes, and use perror instead of printf

:white_check_mark: Move cx's to zoo directory

:white_check_mark: Remove mcx_selector in favor of arrays of function pointers

:white_check_mark: rename instances of cfu to cx

:white_check_mark: Add checks to cx_open / cx_close for robustness in multi-threaded scenarios