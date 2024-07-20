#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "../../../../research/riscv-tools/cx_runtime/include/ci.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/mulacc/mulacc.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/addsub/addsub.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/muldiv/muldiv.h"

#include <sys/types.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h> 

/* 
* Ensuring that the fork function works on whichever device
* or emulator this is run on
*/
void test_fork() { 
  pid_t p = fork(); 
  assert(p >= 0);
  if (p == 0) { 
    exit(EXIT_SUCCESS);
  } else {
    wait(NULL);
  }
  return; 
}

/*
* Open and closing in a single process
*/
void test_fork_0() { 
  int a = 3;
  int b = 5;
  int c = 2;
  int result;

  cx_error_t cx_error;
  uint cx_status;
  int32_t state_result;
  cx_sel_t cx_index, cx_index_0;

  cx_stctxs_t expected_stctxs = {.sel = {
                                  .cs = CX_DIRTY,
                                  .error = 0,
                                  .initializer = CX_HW_INIT,
                                  .state_size = 1
                                }};
  pid_t p = fork(); 
  assert(p >= 0);

  if (p == 0) { 
  // child process
    int cx_sel_C0 = cx_open(CX_GUID_MULACC, EXCLUDED);
    assert(cx_sel_C0 > 0);
    cx_error_clear();
    cx_sel(cx_sel_C0);
    result = mac(b, b);
    assert( result == 25 );
    cx_close(cx_sel_C0);

    exit(EXIT_SUCCESS);
  
  } else {
  // parent process

    wait(NULL);
  }
  return; 
}


/*
* Open and closing in both processes
*/
void test_fork_1() { 
  int a = 3;
  int b = 5;
  int c = 2;
  int result;

  cx_error_t cx_error;
  uint cx_status;
  int32_t state_result;
  cx_sel_t cx_index, cx_index_0;

  cx_stctxs_t expected_stctxs = {.sel = {
                                  .cs = CX_DIRTY,
                                  .error = 0,
                                  .initializer = CX_HW_INIT,
                                  .state_size = 1
                                }};
  pid_t p = fork(); 
  assert(p >= 0);

  if (p == 0) { 
  // child process
    int cx_sel_C0 = cx_open(CX_GUID_MULACC, EXCLUDED);
    assert(cx_sel_C0 > 0);
    cx_error_clear();
    cx_sel(cx_sel_C0);
    result = mac(b, b);
    assert( result == 25 );
    cx_close(cx_sel_C0);

    exit(EXIT_SUCCESS);
  
  } else {
  // parent process
    int cx_sel_C1 = cx_open(CX_GUID_MULACC, EXCLUDED);
    assert(cx_sel_C1 > 0);
    cx_error_clear();
    cx_sel(cx_sel_C1);
    result = mac(c, c);
    assert( result == 4 );
    cx_close(cx_sel_C1);

    wait(NULL);
  }
  return; 
}

/* 
* Strictly stateless open and close
*/
void test_fork_2() {
  int a = 3;
  int b = 5;
  int c = 2;
  int result;

  // cx_share_t EX = 0, share_C = 0;
  cx_error_t cx_error;
  uint cx_status;
  int32_t state_result;
  cx_sel_t cx_index, cx_index_0;

  cx_stctxs_t expected_stctxs = {.sel = {
                                  .cs = CX_DIRTY,
                                  .error = 0,
                                  .initializer = CX_HW_INIT,
                                  .state_size = 1
                                }};

  pid_t pid = fork();

  // cx_index = cx_csr_read(CX_INDEX);
  if (pid < 0){ 
    perror("fork fail"); 
    exit(1); 
  } else if (pid == 0) {
    uint cx_sel_C2 = cx_open(CX_GUID_ADDSUB, EXCLUDED);

    cx_sel(cx_sel_C2);
    result = add(a, b);
    assert( result == 8 );
    result = add(a, c);
    assert( result == 5 );

    cx_close(cx_sel_C2);

    exit(EXIT_SUCCESS);
  } else {
    uint cx_sel_C4 = cx_open(CX_GUID_MULDIV, EXCLUDED);
    uint cx_sel_C5 = cx_open(CX_GUID_ADDSUB, EXCLUDED);

    cx_sel(cx_sel_C4);

    result = mul(a, b);
    assert( result == 15 );
    result = mul(c, b);
    assert( result == 10 );
    result = mul(a, c);
    assert( result == 6 );

    cx_sel(cx_sel_C5);
    result = add(a, b);
    assert(result == 8);

    cx_close(cx_sel_C4);
    cx_close(cx_sel_C5);
    // Parent process
    int status;
    // Wait for child
    // waitpid(pid, &status, 0);
  }
}


/*
* Open and closing in both processes, with some more
* complexities.
*/
void test_fork_3() {
  int a = 3;
  int b = 5;
  int c = 2;
  int result;

  // cx_share_t EX = 0, share_C = 0;
  cx_error_t cx_error;
  uint cx_status;
  int32_t state_result;
  cx_sel_t cx_index, cx_index_0;

  cx_stctxs_t expected_stctxs = {.sel = {
                                  .cs = CX_DIRTY,
                                  .error = 0,
                                  .initializer = CX_HW_INIT,
                                  .state_size = 1
                                }};

  pid_t pid = fork();

  // cx_index = cx_csr_read(CX_INDEX);
  if (pid < 0){ 
    perror("fork fail"); 
    exit(1); 
  } else if (pid == 0) {
    uint cx_sel_C1 = cx_open(CX_GUID_MULACC, EXCLUDED);
    uint cx_sel_C2 = cx_open(CX_GUID_ADDSUB, EXCLUDED);
    cx_sel(cx_sel_C1);

    result = mac(b, c);
    assert( result == 10 );
    result = mac(b, c);
    assert( result == 20 );
    result = mac(b, c);
    assert( result == 30 );

    cx_sel(cx_sel_C2);
    result = add(a, b);
    assert( result == 8 );
    result = add(a, c);
    assert( result == 5 );

    cx_sel(cx_sel_C1);
    result = mac(a, c);
    assert( result == 36 );

    cx_close(cx_sel_C1);
    cx_close(cx_sel_C2);

    exit(EXIT_SUCCESS);
  } else {
    // uint cx_sel_C3 = cx_open(CX_GUID_MULACC, EXCLUDED);
    uint cx_sel_C4 = cx_open(CX_GUID_MULDIV, EXCLUDED);
    uint cx_sel_C5 = cx_open(CX_GUID_ADDSUB, EXCLUDED);

    cx_sel(cx_sel_C4);

    result = mul(a, b);
    assert( result == 15 );
    result = mul(c, b);
    assert( result == 10 );
    result = mul(a, c);
    assert( result == 6 );

    cx_sel(cx_sel_C5);
    result = add(a, b);
    assert(result == 8);

    cx_close(cx_sel_C4);
    cx_close(cx_sel_C5);
    // Parent process
    int status;
    // Wait for child
    // waitpid(pid, &status, 0);
  }
}

void complex_fork_test() {
  int a = 3;
  int b = 5;
  int c = 2;
  int result;

  // cx_share_t EX = 0, share_C = 0;
  cx_error_t cx_error;
  uint cx_status;
  int32_t state_result;
  cx_sel_t cx_index, cx_index_0;

  cx_stctxs_t expected_stctxs = {.sel = {
                                  .cs = CX_DIRTY,
                                  .error = 0,
                                  .initializer = CX_HW_INIT,
                                  .state_size = 1
                                }};
  int cx_sel_C0 = cx_open(CX_GUID_MULACC, PROCESS_SHARED);

  assert(cx_sel_C0 > 0);

  cx_error_clear();
  cx_sel(cx_sel_C0);
  result = mac(b, b);
  assert( result == 25 );
  
  cx_error = cx_error_read();
  assert( cx_error == 0 );
  
  cx_status = CX_READ_STATUS();
  assert( cx_status == expected_stctxs.idx );

  pid_t pid = fork();
  if (pid < 0){ 
    perror("fork fail"); 
    exit(1); 
  } else if (pid == 0) {
    uint cx_sel_C1 = cx_open(CX_GUID_MULACC, EXCLUDED);
    uint cx_sel_C2 = cx_open(CX_GUID_ADDSUB, EXCLUDED);
    cx_sel(cx_sel_C1);

    result = mac(b, c);
    assert( result == 10 );
    result = mac(b, c);
    assert( result == 20 );
    result = mac(b, c);
    assert( result == 30 );

    cx_sel(cx_sel_C2);
    result = add(a, b);
    assert( result == 8 );
    result = add(a, c);
    assert( result == 5 );

    cx_sel(cx_sel_C1);
    result = mac(a, c);
    assert( result == 36 );

    cx_close(cx_sel_C1);
    cx_close(cx_sel_C2);

    exit(EXIT_SUCCESS);
  } else {
    // uint cx_sel_C3 = cx_open(CX_GUID_MULACC, EXCLUDED);
    uint cx_sel_C4 = cx_open(CX_GUID_MULDIV, EXCLUDED);
    uint cx_sel_C5 = cx_open(CX_GUID_ADDSUB, EXCLUDED);

    cx_sel(cx_sel_C4);

    result = mul(a, b);
    assert( result == 15 );
    result = mul(c, b);
    assert( result == 10 );
    result = mul(a, c);
    assert( result == 6 );

    cx_sel(cx_sel_C5);
    result = add(a, b);
    assert(result == 8);

    cx_close(cx_sel_C4);
    cx_close(cx_sel_C5);
    // Parent process
    int status;
    // Wait for child
    // waitpid(pid, &status, 0);
  }

  result = mac(b, b);

  cx_close(cx_sel_C0);

  cx_sel(CX_LEGACY);
}

int main() {
    cx_sel(CX_LEGACY);
    test_fork();
    test_fork_0();
    test_fork_1();
    test_fork_2();
    test_fork_3();
    complex_fork_test();
    printf("Context Copy Test Complete\n");
    return 0;
}
