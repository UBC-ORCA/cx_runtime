# Short Term Objectives

:white_check_mark: Move cx's to zoo directory

:white_check_mark: Remove mcfu_selector in favor of arrays of function pointers

:x: rename instances of cfu to cx

:x: Create CX_STATUS CSR to keep track of errors (%2.2.2)

:x: Add checks to cx_open / cx_close for robustness in multi-threaded scenarios

:x: Define error codes, and use perror instead of printf

:x: Check objdump to make sure that inlined functions are actually being inlined

# Medium Term Objectives

:x: Add `probe` operation to check if a given cx can properly be accessed

:x: Create a CI / CD test harness

:x: Consider how the user will interact with the `cx_error();` function - when will it be called? 
    should it be a part of `cx_close();`?