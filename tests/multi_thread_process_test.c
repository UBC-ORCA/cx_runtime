#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../../../research/riscv-tools/cx_runtime/include/ci.h"
#include "../../../../research/riscv-tools/cx_runtime/zoo/mulacc/mulacc.h"

#include <sys/types.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>


static int a = 3, b = 5, c = 2;
// static cx_stctxs_t expected_stctxs = {.sel = {
//                                 .cs = CX_DIRTY,
//                                 .error = 0,
//                                 .initializer = CX_HW_INIT,
//                                 .state_size = 1
//                               }};

void* cx_mac_thread(void *ptr) {
    int val = *(int *)ptr;
    mac(val, val);
    return NULL;
}

void basic_multiP_multiT() { 

  int N = 10;

  cx_error_t cx_error;
  uint cx_status;
  int32_t state_result;
  cx_sel_t cx_index, cx_index_0;

  cx_sel_t cx_sel_0 = cx_open(CX_GUID_MULACC, CX_FULL_VIRT, -1);
  assert(cx_sel_0 > 0);

  pthread_t tid[ N ];
  void *ret;

  cx_sel_t cx_sel_A0 = cx_open(CX_GUID_MULACC, CX_FULL_VIRT, -1);
  assert(cx_sel_A0 > 0);
  cx_sel(cx_sel_A0);

  pid_t pid = fork();
  assert(pid >= 0);

  if (pid == 0) { 
  // child process
    int *ptr = malloc(sizeof(int));
    *ptr = b;
    for (int i = 0; i < N; i++) {
        assert( pthread_create(&tid[i], NULL, cx_mac_thread, ptr) == 0 );
    }

    for (int i = 0; i < N; i++) {
        assert( pthread_join(tid[i], &ret) == 0 );
    }

    int res = CX_READ_STATE(0);
    assert( res == N * b * b);
    assert(cx_csr_read(CX_STATUS) == 0);
    cx_close(cx_sel_A0);
    cx_close(cx_sel_0);

    exit(EXIT_SUCCESS);
  
  } else {
  // parent process
    int *ptr = malloc(sizeof(int));
    *ptr = a;
    for (int i = 0; i < N; i++) {
        assert( pthread_create(&tid[i], NULL, cx_mac_thread, ptr) == 0 );
    }

    for (int i = 0; i < N; i++) {
        assert( pthread_join(tid[i], &ret) == 0 );
    }

    int res = CX_READ_STATE(0);
    assert( res == N * a * a);
    assert(cx_csr_read(CX_STATUS) == 0);
    cx_close(cx_sel_A0);

    int status;
    // Wait for child
    waitpid(pid, &status, 0);
    assert(status == 0);
  }
  cx_close(cx_sel_0);
  cx_sel(CX_LEGACY);

  return; 
}

int main() {
    cx_sel(CX_LEGACY);
    basic_multiP_multiT();
    printf("Finished Multi-thread Multi-process test\n");
    return 0;
}