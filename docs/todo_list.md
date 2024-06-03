# Short Term Objectives

:white_check_mark: Fix stateless cxs

:x: Add tests, making sure that the correct state off / init / dirty / clean values are set correctly

:white_check_mark: Fix m mode cx_{open, close, sel}, so that it works the same as u mode

:x: Document the functionality of cx_{open, close, sel}

:x: Fix builds

:x: Trap on write to mcx_selector, so that it writes to the correct address (0xBC0 instead of 0x802)

:x: Create CX_STATUS CSR to keep track of errors (%2.2.2)

:x: Check objdump to make sure that inlined functions are actually being inlined

:x: replace instances of setting errno with first getting the local errno with `__errno_location(void)`.

# Medium Term Objectives

:x: Create a CI / CD test harness

:x: Add `probe` operation to check if a given cx can properly be accessed

:x: Consider how the user will interact with the `cx_error();` function - when will it be called? 
    should it be a part of `cx_close();`?

# Completed objectives

:white_check_mark: Move structs in cfu_helper to be initialized elsewhere + add padding

:white_check_mark: Define error codes, and use perror instead of printf

:white_check_mark: Move cx's to zoo directory

:white_check_mark: Remove mcx_selector in favor of arrays of function pointers

:white_check_mark: rename instances of cfu to cx

:white_check_mark: Add checks to cx_open / cx_close for robustness in multi-threaded scenarios
